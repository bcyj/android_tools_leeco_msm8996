/******************************************************************************
@file  fbtest.c
@brief This file contains test code to verify all functionalities of msm_fb

DESCRIPTION
fbtest is msm framebuffer test program.  It opens all frambuffers (/dev/fb*)
and executes the msm specific fb ioctls as well as the standard linux fb
ioctls.

-----------------------------------------------------------------------------
Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------------

******************************************************************************/
#include "fbtest.h"
#include "fbtestUtils.h"
#ifdef ENABLE_POSTPROC
#include "lib-postproc.h"
#endif
#ifdef MDP3_FLAG
#include "mdp3.h"
#endif
#ifdef MDP4_FLAG
#include "mdp4.h"
#endif

#define MAX_FRAME_BUFFER_NAME_SIZE 80

int fbtest_msg_debug = 0;	/* set non-zero for printing debugging messages*/
static int path;
tests testFunctions[MAX_TESTS];
struct lookup_string_t testNameLUT[MAX_TESTS];

#ifdef MDP3_FLAG
extern int MDP3TestFuncArraySize;
extern int MDP3TestNameArraySize;
extern tests MDP3testFunctions[];
extern struct lookup_string_t MDP3testNameLUT[];
#endif

#ifdef MDP4_FLAG
extern int MDP4TestFuncArraySize;
extern int MDP4TestNameArraySize;
extern tests MDP4testFunctions[];
extern struct lookup_string_t MDP4testNameLUT[];
#endif

/***********************************************************
 Function Definitions
***********************************************************/
int chooseFBDev(int choice)
{
	int i, result,count;
	int fb_dev, fb_name_len;
	char fb_name[FB_NAME_STR_LEN];
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	// Check if framebuffer exists
	fb_dev = open(FB_NAME_STR_LE, O_RDWR);
	if (fb_dev < 0) {
		fb_dev = open(FB_NAME_STR_LA, O_RDWR);
		if (fb_dev < 0) {
			return -FBDEV_NOT_FOUND;
		} else {
			close(fb_dev);
			strlcpy(fb_name, FB_NAME_STR_LA, sizeof(fb_name));
		}
	} else {
		close(fb_dev);
		strlcpy(fb_name, FB_NAME_STR_LE, sizeof(fb_name));
	}

	if (choice >= 0)
		result = choice;
	else {
		printf("Please select the target framebuffer:\n");
		for (i = 0; i < FB_NUM_MAX; i++)
			printf("  %d. %s%d\n", i+1, fb_name, i);

		printf("-> ");
		result = read_int();
		if ((result <= 0) || (result > FB_NUM_MAX)) {
			printf("ERROR: Invalid choice!\n");
			return -1;
		}
		result--;
	}

	result = thisFBTEST->fBuffer;

	if (result < FB_NUM_MAX) {
		/* a single framebuffer selected */
		FB = &(fb[result]);
		FB->fb_num = result;
		fb_name_len = strlcpy(FB->fb_name, fb_name, sizeof(FB->fb_name));
		FB->fb_name[fb_name_len - 1] = 0x30+result;
		FBLIST_GLOBAL[0] = FB;
		FBLISTSIZE_GLOBAL = 1;
	} else {
		/* more than one framebuffer selected */
		for (count = 0; count < FB_CONCURRENT_NUM; count++) {
			FBLIST_GLOBAL[count] = &(fb[count]);
			FBLIST_GLOBAL[count]->fb_num = count;
			fb_name_len = strlcpy(FBLIST_GLOBAL[count]->fb_name, fb_name, sizeof(FBLIST_GLOBAL[count]->fb_name));
			FBLIST_GLOBAL[count]->fb_name[fb_name_len - 1] = 0x30+count;
		}
		FBLISTSIZE_GLOBAL = FB_CONCURRENT_NUM;
	}

	return 0;
}

int chooseMEMDev()
{
#ifdef USE_ION
	ION = &(ion[ION_NUM_DEFAULT]);
	strlcpy(ION->mem_name, ION_NAME_STR, sizeof(ION->mem_name));
#else
	int pmem_dev,pmem_dev2;
	char pmem_name[PMEM_NAME_STR_LEN];

	// Check if PMEM exists
	pmem_dev = open(PMEM_NAME_STR, O_RDWR);
	if (pmem_dev < 0) {
		pmem_dev2 = open(PMEM_NAME_STR2, O_RDWR);
		if (pmem_dev2 < 0) {
			return -PMEMDEV_NOT_FOUND;
		} else {
			close(pmem_dev2);
			strlcpy(pmem_name, PMEM_NAME_STR2, sizeof(pmem_name));
		}
	} else {
		close(pmem_dev);
		strlcpy(pmem_name, PMEM_NAME_STR, sizeof(pmem_name));
	}

	PMEM = &(pmem[PMEM_NUM_DEFAULT]);
	strlcpy(PMEM->mem_name, pmem_name, sizeof(PMEM->mem_name));
#endif
	return 0;
}

/* autoselect = 1 => Auto select based on selected FB device
 autoselect = 0 & choice = -1 => manual selection*/
int chooseFBFmt(int choice, struct fbDev *thisFB, int autoselect)
{
	int result;
	int len = strlen("msmfbXX_") + 1;
	int bpp = thisFB->fb_vinfo.bits_per_pixel / 8;
	FBTEST_MSG_DEBUG("%s, choice == %d, autoselect == %d\n", __func__, choice, autoselect);
	if (autoselect) {
		switch (panel_type) {
		case PANEL_PRIMARY_LCD:
		case PANEL_SECONDARY_LCD:
			result = FB_FMT_PANEL;
			if (bpp == 4)
				result = MDP_RGBA_8888;
			else if (mdp_version >= MDP_V4_0) {
				if (bpp == 3)
					result = MDP_RGB_888;
				else if (bpp == 2)
					result = MDP_RGB_565;
			}
			break;
		case PANEL_TVOUT:
			result = FB_FMT_TVOUT;
			break;
		default:
			FBTEST_MSG_DEBUG("%s: choosing FB_FMT_DEFAULT\n", __func__);
			result = FB_FMT_DEFAULT;
			break;
		}
	} else {
		if (choice < 0) {
			printf("Please select color format:\n");
			printf("   %d) MDP_RGB_565\n",MDP_RGB_565);
			printf("   %d) MDP_RGB_888\n",MDP_RGB_888);
			printf("   %d) MDP_ARGB_8888\n",MDP_ARGB_8888);
			printf("   %d) MDP_RGBA_8888\n",MDP_RGBA_8888);
			printf("   %d) MDP_Y_CBCR_H2V2\n",MDP_Y_CBCR_H2V2);
			printf("   > ");

			result = read_int();
		}
		else
			result = choice;

		if ((result != MDP_RGB_565) &&
			(result != MDP_RGB_888) &&
			(result != MDP_ARGB_8888) &&
			(result != MDP_RGBA_8888) &&
			(result != MDP_Y_CBCR_H2V2)) {
			printf("ERROR: Invalid choice!\n");
			result = -1;
		}
	}

	if (result >= 0) {
		thisFB->fb_fmt = result;
		FBTEST_MSG_DEBUG("%s: result is %d\n", __func__, result);
	}
	else
		printf("ERROR: Color format not modified.\n");


	return 0;
}

int openFBDev(struct fbDev *thisFB)
{
	thisFB->fb_fd = open(thisFB->fb_name, O_RDWR);
	if (thisFB->fb_fd < 0) {
		FBTEST_MSG_DEBUG("[%s(%d)]: %s opened FB failed!\n", __func__ , __LINE__, thisFB->fb_name);
		return -FBDEV_OPEN_FAILED;
	}
	else
		FBTEST_MSG_DEBUG("[%s(%d)]: %s opened FB successfully!\n", __func__ , __LINE__, thisFB->fb_name);

        if (brightness_change(BRIGHTNESS_STORE_NEW_LEVEL) < 0) {
                close(thisFB->fb_fd);
                return -1;
        }

	return 0;
}

int openMEMDev()
{
#ifdef USE_ION
        ION->fd = open(ION->mem_name, O_RDWR|O_DSYNC);
        if (ION->fd < 0) {
                printf("ERROR: Can't open ion %s\n", ION->mem_name);
                return -IONDEV_OPEN_FAILED;
	} else {
		FBTEST_MSG_DEBUG("[%s(%d)]: %s opened ION successfully!\n", __func__ , __LINE__, ION->mem_name);
	}
        MEM = ION;
#else
        PMEM->fd = open(PMEM->mem_name, O_RDWR);
        if (PMEM->fd < 0) {
                printf("ERROR: Can't open pmem %s\n", PMEM->mem_name);
                return -PMEMDEV_OPEN_FAILED;
	} else {
		FBTEST_MSG_DEBUG("[%s(%d)]: %s opened PMEM successfully!\n", __func__ , __LINE__, PMEM->mem_name);
	}
        MEM = PMEM;
#endif
        return 0;
}

int getFBInfo(struct fbDev *thisFB)
{
	if (ioctl(thisFB->fb_fd, FBIOGET_VSCREENINFO, &(thisFB->fb_vinfo)) < 0) {
		closeFBDev(thisFB);
		return -VSCREEN_GET_INFO_FAILED;
	}
	thisFB->fb_vinfo.yoffset = 0;
	thisFB->fb_vinfo.reserved[0] = 0;

	if (ioctl(thisFB->fb_fd, FBIOGET_FSCREENINFO, &(thisFB->fb_finfo)) < 0) {
		closeFBDev(thisFB);
		return -FSCREEN_GET_INFO_FAILED;
	}

	thisFB->fb_size = ((thisFB->fb_finfo.line_length * 8) / thisFB->fb_vinfo.bits_per_pixel) *
				thisFB->fb_vinfo.yres * thisFB->fb_vinfo.bits_per_pixel / 8;

	FBTEST_MSG_DEBUG("thisFB->fb_size: %d, thisFB->fb_finfo.line_length: %d, thisFB->fb_vinfo.yres: %d\n",\
				thisFB->fb_size, thisFB->fb_finfo.line_length, thisFB->fb_vinfo.yres);

	return 0;
}

int getMDPInfo(struct fbDev *thisFB)
{
	FBTEST_MSG_DEBUG("FB->fb_finfo.id %s\n", thisFB->fb_finfo.id);
	char *fbid = thisFB->fb_finfo.id;

	mdp_version = MDP_V_UNKNOWN;

	if (!strncmp(fbid, "msmfb", 5)) {
		char str_ver[4] = { 0 };
		memcpy(str_ver, &fbid[5], 3);
		str_ver[3] = '\0';
		mdp_version = atoi(str_ver);
		/* Normalize MDP version to ease comparison.
		   This is needed only because MDP 3.0.3 reports
		   value as 303 which is more than all the others */
		if (mdp_version < 100)
			mdp_version *= 10;
	} else if (!strncmp(fbid, "mdssfb", 6)) {
#ifdef MDP3_FLAG
		mdp_version = MDP_V3_0;
#else
		mdp_version = MDP_V5;
#endif
		struct msmfb_metadata metadata;
		memset(&metadata, 0x00 , sizeof(metadata));
		metadata.op = metadata_op_get_caps;
		if (ioctl(thisFB->fb_fd, MSMFB_METADATA_GET, &metadata) == -1) {
			printf("Error retrieving MDP revision\n");
			mdp_rev = MDP_V_UNKNOWN;
		} else {
			mdp_rev = metadata.data.caps.mdp_rev;
		}
	}
	return mdp_version;
}

int getPanelInfo(struct fbDev *thisFB)
{
	char *pan_id;
	pan_id = strchr(thisFB->fb_finfo.id, '_') + 1;
	if (!pan_id) {
		panel_type = PANEL_UNKNOWN;
		return 0;
	}

	FBTEST_MSG_DEBUG("%s: fb_finfo.id == \"%s\"\n", __func__,
				thisFB->fb_finfo.id);

	switch (*pan_id) {
	case MDDI_PANEL:
	case LCDC_PANEL:
	case EBI2_PANEL:
	case EXT_MDDI_PANEL:
	case MIPI_VIDEO_PANEL:
	case MIPI_CMD_PANEL:
	case DTV_PANEL:
	case LVDS_PANEL:
	case EDP_PANEL:
		panel_type = PANEL_PRIMARY_LCD;
		break;
	case WRITEBACK_PANEL:
		panel_type = PANEL_PRIMARY_LCD;
		writeback_flag = TRUE;
		break;
	case TV_PANEL:
		panel_type = PANEL_TVOUT;
		break;
	default:
		panel_type = PANEL_UNKNOWN;
		break;
	}
	return 0;
}

int mapFB(struct fbDev *thisFB)
{
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int mapsize;

	mapsize = FB->fb_finfo.line_length * FB->fb_vinfo.yres_virtual;

	thisFB->fb_buf = thisFB->fb_buf_back = mmap(NULL,
		mapsize, PROT_READ | PROT_WRITE,
		MAP_SHARED, thisFB->fb_fd, 0);

	if (thisFB->fb_buf == MAP_FAILED) {
		closeFBDev(thisFB);
		return -MMAP_FAILED;
	} else {
		FBTEST_MSG_DEBUG("Framebuffer mmap successful (%d bytes at %p)\n", thisFB->fb_finfo.smem_len, thisFB->fb_buf);
		thisFB->fb_buf_back += thisFB->fb_size;
	}
	return 0;
}

int closeFBDev(struct fbDev *thisFB)
{
	int result;

	if (brightness_change(BRIGHTNESS_RESTORE_PREV_LEVEL) < 0)
		printf("Cannot restore previous brightness level\n");

	result = close(thisFB->fb_fd);
	if (result < 0) {
		printf("ERROR: Can't close framebuffer %s!\n", thisFB->fb_name);
		return -1;
	} else {
		printf("%s closed successfully!\n", thisFB->fb_name);
		return 0;
	}
}

// Helper Functions
int read_int(void)
{
	int input;
	char c;

	input = 0;
	while ((c = getchar()) != '\n')
		input = input*10+ ((c < 0x30)?0:c-0x30);

	return input;
}

void swapPointers(unsigned char **a, unsigned char **b)
{
	unsigned char *temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

void showUsage(void) {

	printf("\n%s : Show working fbtest usage and options\n\n",VERSION);
	printf("\n./fbtest -t testname/number [parameters].\n\n");

	printf("  -u,						--usage		print this usage and exit\n");
	printf("  -V,						--Version	print version and exit\n");
	printf("  -v,						--verbose	set verbose flag\n\n");
	printf("  -n [nominal test],				--nominal  	run nominal test\n\n");
	printf("  -a [adversarial test], 			--adversarial	run adversarial test\n\n");
	printf("  -r [repetability test], 			--repetability  run repetability test\n\n");
	printf("  -s [stress test], 				--stress        run stress test\n\n");

	printf("  -C [CONFIG FILE],         			--config      	configuration file location\n");
	printf("  -b [framebuffer {fb0, fb1}], 			--fb    	framebuffer to choose\n");
	printf("  -m [MODE {user, auto, negative}], 		--mode    	mode to run the test\n");
	printf("  -p [FILE or DIRECTORY PATH], 			--filepath     	set input file or direcotry path\n");
	printf("  -D [mixer-2 dump DIRECTORY PATH], 		--dumpPath     	set mixer-2 dump direcotry path\n");
	printf("  -t [TEST {testname or number}], 		--test         	test to run\n");
	printf("  MDP4 tests: allmdp4[0], format[1], scale[2], rotate[3], crop[4], move[5], deinterlace[6], dither[7], videoplay[8], fpsvsync[9], blend[10], colorkey[11], multiop[12], adversarial[13],hwcursor[14], overlayscale[15], overlayalpha[16], overlayStress[17],allcolorFormatOverlay[18],overlayARGB[19],overlay[20],csc[21]\n\n");
	printf("  MDP3 tests: allmdp3[0], format[1], scale[2], rotate[3], crop[4], move[5], dither[6], videoplay[7], fpsvsync[89], blend[9], colorkey[10], multiop[11], adversarial[12], overlay[13], csc[14], postproc[15], drawbg[16], calcfps[17], ppp[18], allColorformatPPP[19], pppTestVideo[20], stress[21], mddiPartial[22] \n\n");

	printf(" option [TEST {parameter}], 			--long option   Description\n");
	printf("----------------------------------------------------------------------------\n");
	printf("  -w [SCALE {Output width}], 			--outwidth     	set output width for scaling\n");
	printf("  -h [SCALE {Output height}], 			--outheight    	set output height for scaling\n");
	printf("  -S [SCALE-STEP {0, 1}], 		 	--scalestep    	set scale in steps\n\n");

	printf("  -d [ROTATE {0, 90, 180, 270}], 	 	--degree       	rotate value in Degree\n");
	printf("  -f [ROTATE {NOFLIP, LR, UD, LRUD/UDLR}], 	--flip         	set flip mode\n\n");

	printf("  -c [CROP {25, 50, 75, LEFT, RIGHT, TOP, BOTTOM, RAND}], --crop_p set crop mode\n");
	printf("  -H [CROP {Crop height}]			--cropheight    set cropping height\n");
	printf("  -W [CROP {Crop width}]                        --cropwidth     set cropping width\n");
	printf("  -i [CROP RAND x-coordinate], 			--coordinate_x 	set crop x coordinate\n");
	printf("  -j [CROP RAND y-coordinate], 			--coordinate_y 	set crop y coordinate\n\n");

	printf("  -x [MOVE startX], 				--startX       	set move source x coordinate\n");
	printf("  -y [MOVE startY], 				--startY       	set move source y coordinate\n");
	printf("  -X [MOVE endX], 				--endX       	set move destination x coordinate\n");
	printf("  -Y [MOVE endY], 				--endY       	set move destination y coordinate\n\n");
	printf("  -F [VIDEO fps], 				--fps       	Play video file with given fps value\n\n");

	printf("  -1 [BLEND/COLORKEY  BG file path], 		--bgimage	BG Image for blend or colorkey test\n");
	printf("  -2 [BLEND/COLORKEY  FG1 file path], 		--fg1image	First FG Image for blend or colorkey test\n");
	printf("  -3 [BLEND  FG2 file path],	 		--fg2image	Second FG Image for blend or colorkey test\n");
	printf("  -4 [BLEND  FG3 file path], 			--fg3image	Third FG Image for blend or colorkey test\n");
	printf("  -P [BLEND alpha value],	 		--fg1alpha	Alpha value for FG1 input image file\n");
	printf("  -Q [BLEND alpha value], 			--fg2alpha	Alpha value for FG2 input image file\n");
	printf("  -R [BLEND alpha value], 			--fg3alpha	Alpha value for FG3 input image file\n");
	printf("  -U [BLEND alpha value],			--fg4alpha	Alpha value for FG4 input image file\n");
	printf("  -T [COLORKEY transperancy value], 		--fg3image	Third FG Image for blend or colorkey test\n");
	printf("  pix/const [BLEND], 				pix- Enable pixel blending, const- Enable const blending (default is const)\n");
	printf("  bg/fg [COLORKEY], 				bg - background colorkey, fg- foreground colorkey\n\n");

	printf("  on/off [DITHER/DE-INTERLACE], 		enable/disable dither or de-Interlace flag\n\n");

	printf("  msc/rmsc [MULTIOP], 				msc - move-scale-crop multi-operation test, rmsc- rotate-move-crop-scale multi-operation test\n\n");
	printf("  -k [],					-k - run test in interactive mode\n\n");
	printf("  -A,						-A sets the async flag\n");
	printf("  -O [MOVE type],				--type LR/UD/X/RND \n\n");
	printf("  -Z ,						--type copies destination buffer to a file \n\n");
	printf("  misr,						misr - MISR based validation\n");
	printf("  hist,						hist - Histogram Based Validation\n");
	printf("  crc [0/1],					crc - crc0 used for CRC data base creation & crc1 for validation\n");
	printf("  -e [],					-e - CRC file path\n");
	printf("  decimation					Enable decimation\n");
	exit(0);
}

int parseFileName(const char *file, struct inputFileParams *thisInputFileParams)
{
	char *ptr;
	int  flag = 0;//to check 'x' present in resolution
	char* resolution;
	char* format;
	char tmp[25];
	char* filename;
	int shiftCountByName = 0;

	//take a copy of file name to use in different test
	strlcpy(thisInputFileParams->filenamePath, file, sizeof(thisInputFileParams->filenamePath));
	//if file name contains directory path also like /data/kernel-test/black_320x420_RGB_565.rgb
	//then escape till start of actual file name.
	ptr = strrchr(file, '/');
	if (ptr) {
		ptr++;
	} else {
		ptr = (char *)file;
	}
	filename = strtok(ptr, "_");
	if (filename == NULL)
		return -PARSE_FAILED;
	strlcpy(thisInputFileParams->filename, filename, sizeof(thisInputFileParams->filenamePath));

	resolution = strtok(NULL,"_");
	if (resolution == NULL)
		return -PARSE_FAILED;
	format = strtok(NULL, ".");
	if (format == NULL)
		return -PARSE_FAILED;
	FBTEST_MSG_DEBUG("filename %s \t resolution %s\tformat %s \n", filename, resolution, format);

	//validate input format
	strlcpy(tmp, "MDP_", sizeof(tmp));
	strlcat(tmp, format, sizeof(tmp));

	thisInputFileParams->inputFormat = lookup_key_case_insensitive(format_name_table, tmp, -1);
	if (thisInputFileParams->inputFormat == -1) {
		FBTEST_MSG_DEBUG("Not a valid Input format (%s)\n", tmp);
		return -INVALID_FORMAT;
	}

	strlcpy(thisInputFileParams->charFormat, format, sizeof(thisInputFileParams->charFormat));


	ptr =NULL;
	ptr = resolution;
	//validate resolution enter in correct format
	while (strlen(ptr)) {
		if ((*ptr >= '0' && *ptr <= '9') || (*ptr == 'x')) {
			if (*ptr == 'x') {
				flag = 1;
			}
			ptr ++;
		} else {
			FBTEST_MSG_DEBUG("resolution part of filename is not in correct format\n");
			return -PARSE_FAILED;
		}
	}
	if (flag) {
		ptr = resolution;
		thisInputFileParams->inResolution.width = atoi(strtok(ptr, "x"));
		thisInputFileParams->inResolution.height = atoi(strtok(NULL,"\0"));
		if ((thisInputFileParams->inResolution.width <= FB->fb_vinfo.xres) &&
			(thisInputFileParams->inResolution.height <= FB->fb_vinfo.yres)) {
			FBTEST_MSG_DEBUG("format %s\twidth %d\t height %d\n",
							thisInputFileParams->charFormat,
							thisInputFileParams->inResolution.width,
							thisInputFileParams->inResolution.height);
		}
	}else{
		FBTEST_MSG_DEBUG("resolution part does not contain both width & height\n");
		return -PARSE_FAILED;
	}
	return 0;
}

int parse_opt(int argc, char **argv)
{
	int opt, shiftCountByName = -1, shiftCountByNum = -1;
	struct stat sb;
	struct fbtest_params *thisFBTEST;
	char *testName = NULL;
	long testValue = -1;
	int ret = 0;
	int key, val;
	int test_num;
	errno = 0;
	thisFBTEST = &FBTEST;
	interactive = DISABLED;
	struct option longOptions[] = {
        { "usage",		no_argument,       NULL, 'u'},
        { "Version",		no_argument,       NULL, 'V'},
        { "verbose",		no_argument,       NULL, 'v'},
        { "nominal",		no_argument,       NULL, 'n'},
        { "adversarial",	no_argument,       NULL, 'a'},
        { "repeatability",	no_argument,       NULL, 'r'},
        { "stress",		no_argument,       NULL, 's'},
        { "config",		no_argument,       NULL, 'C'},
	{ "interactive",        no_argument,       NULL, 'k'},
        { "fb",			required_argument, NULL, 'b'},
        { "mode",		required_argument, NULL, 'm'},
        { "test",		required_argument, NULL, 't'},
        { "filepath",		required_argument, NULL, 'p'},
        { "dumpPath",		required_argument, NULL, 'D'},
        { "outheight",		required_argument, NULL, 'h'},
        { "outwidth",		required_argument, NULL, 'w'},
        { "scalestep",		required_argument, NULL, 'S'},
        { "degree",		required_argument, NULL, 'd'},
        { "flip",		required_argument, NULL, 'f'},
        { "crop_p",		required_argument, NULL, 'c'},
        { "cropheight",		required_argument, NULL, 'H'},
        { "cropwidth",		required_argument, NULL, 'W'},
        { "coordinate_x",	required_argument, NULL, 'i'},
        { "coordinate_y",	required_argument, NULL, 'j'},
        { "startX",		required_argument, NULL, 'x'},
        { "startY",		required_argument, NULL, 'y'},
        { "endX",		required_argument, NULL, 'X'},
        { "endY",		required_argument, NULL, 'Y'},
        { "fps",		required_argument, NULL, 'F'},
        { "bgimage",		required_argument, NULL, '1'},
        { "fgimage1",		required_argument, NULL, '2'},
        { "fgimage2",		required_argument, NULL, '3'},
        { "fgimage3",		required_argument, NULL, '4'},
        { "fg1Alpha",		required_argument, NULL, 'P'},
        { "fg2Alpha",		required_argument, NULL, 'Q'},
        { "fg3Alpha",		required_argument, NULL, 'R'},
        { "fg4Alpha",		required_argument, NULL, 'U'},
        { "transparency",	required_argument, NULL, 'T'},
        { "iteration",		required_argument, NULL, 'I'},
        { "file",               required_argument, NULL, 'B'},
	{ "moveop",		required_argument, NULL, 'O'},
	{ "async_flag",		no_argument,       NULL, 'A'},
	{ "dest_dump",		no_argument,       NULL, 'Z'},
        { NULL,			0,                 NULL,  0 },
	};

	optind = 0;

	/*
	// no arguments given
	*/
	if (argc == 1) {
		fprintf(stderr, "This program needs arguments....\n\n");
		showUsage();
	}
	thisFBTEST->coordinate.x = 0;
	thisFBTEST->coordinate.y = 0;
	/*
	   print all remaining options
	*/
	strlcpy(thisFBTEST->crcPath, "/data/", sizeof(thisFBTEST->crcPath));
	thisFBTEST->crcmode = -1;
	thisFBTEST->misr    = 0;
	thisFBTEST->moveType = DIAGONALS;
	thisFBTEST->decimation = FALSE;

	while ((opt = getopt_long(argc, argv, "uVvnarsb:m:t:p:D:h:w:S:d:r:f:c:H:W:i:j:x:y:X:Y:F:1:2:3:4:P:Q:R:T:U:I:B:k:O:e:AZ", longOptions, NULL)) != -1) {
		unsigned int argSize = 0;
		switch(opt) {
			case 'u':
				showUsage();
				break;
			case 'V':
				printf("%s\n\n", VERSION);
				exit(0);
				break;
			case 'v':
				verbose = ENABLED;
				fbtest_msg_debug = ENABLED;
				break;
#ifdef MDP3_FLAG
			 case 'n':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "fpsvsync", -1);
				shiftCountByNum = lookup_val(testNameLUT, 8, -1);
				testValue = strtol("fpsvsync", &testName, 0);
				thisFBTEST->testname = "fpsvsync";
				key = lookup_key_case_insensitive(testNameLUT, "fpsvsync", -1);
				val = lookup_val(testNameLUT, key, -1);
				testEnable |= 1 << val;
				snprintf(thisFBTEST->blend.ImagePath[0], sizeof("yellow_face.zif"), "%s", "yellow_face.zif");
				snprintf(thisFBTEST->blend.ImagePath[0], sizeof("mdp11_CbCr_10F.zif"), "%s", "mdp11_CbCr_10F.zif");
				 break;
			case 'a':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "adversarial", -1);
				shiftCountByNum = lookup_val(testNameLUT, 12, -1);
				testValue = strtol("adversarial", &testName, 0);
				thisFBTEST->testname = "adversarial";
				key = lookup_key_case_insensitive(testNameLUT, "adversarial", -1);
				val = lookup_val(testNameLUT, key, -1);
				testEnable |= 1 << val;
				thisFBTEST->testMode = USER;
				break;
			case 'r':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "format", -1);
				shiftCountByNum = lookup_val(testNameLUT, 1, -1);
				testValue = strtol("format", &testName, 0);
				thisFBTEST->testname = "format";
				key = lookup_key_case_insensitive(testNameLUT, "format", -1);
				val = lookup_val(testNameLUT, key, -1);
				testEnable |= 1 << val;
				thisFBTEST->testMode = USER;
				break;
			case 's':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "multiop", -1);
				shiftCountByNum = lookup_val(testNameLUT, 11, -1);
				testValue = strtol("multiop", &testName, 0);
				thisFBTEST->testname = "multiop";
				key = lookup_key_case_insensitive(testNameLUT, "multiop", -1);
				val = lookup_val(testNameLUT, key, -1);
				testEnable |= 1 << val;
				thisFBTEST->testMode = USER;
				break;
#endif

#ifdef MDP4_FLAG
			case 'n':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "fpsvsync", -1);
				break;
			case 'a':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "adversarial", -1);
				  break;
			case 'r':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "allcolorFormatOverlay", -1);
				  break;
			case 's':
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "overlayStress", -1);
				  break;
#endif
			case 'b':
				if (!(strcasecmp(optarg, "fb0") && strcmp(optarg, "0")))
					thisFBTEST->fBuffer = FB0;
				else if (!(strcasecmp(optarg, "fb1") && strcmp(optarg, "1")))
					thisFBTEST->fBuffer = FB1;
				else if (!(strcasecmp(optarg, "fb2") && strcmp(optarg, "2")))
					thisFBTEST->fBuffer = FB2;
				else if (!(strcasecmp(optarg, "fb01") && strcmp(optarg, "3")))
					thisFBTEST->fBuffer = FB01;
				else
					showUsage();
				break;
			case 'm':
				if (!(strcasecmp(optarg, "user") && strcmp(optarg, "1")))
					thisFBTEST->testMode = USER;
				else if (!(strcasecmp(optarg, "auto") && strcmp(optarg, "2")))
					thisFBTEST->testMode = AUTO;
				else
					showUsage();
				break;
			case 't':
				testValue = strtol(optarg, &testName, 0);
				if (!testName)
					return -PARSE_FAILED;
				thisFBTEST->testname = optarg;
				switch (errno) {
					case ERANGE:
						FBTEST_MSG_DEBUG("The data could not be represented.\n");
						showUsage();
						break;
					case EINVAL:
						FBTEST_MSG_DEBUG("Unsupported base / radix.\n");
						showUsage();
						break;
				}

				FBTEST_MSG_DEBUG("The value is %ld, leftover part of the string is %s\n",testValue, testName == NULL ? "N/A" : testName);

				if (testValue == 0 && (strcmp(testName, "")))
				{
					testValue = -1;
				}
				test_num = atoi(thisFBTEST->testname);
				if(test_num) {
					thisFBTEST->testname = (char*)lookup_string(testNameLUT,
							test_num, "unknown");
				}
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, testName, -1);
				shiftCountByNum = lookup_val(testNameLUT, testValue, -1);

				break;
			case 'p':
				snprintf(thisFBTEST->path, sizeof(thisFBTEST->path), "%s", optarg);
				if (strcmp(thisFBTEST->path,"")) {
					if (stat(thisFBTEST->path, &sb) == 0) {
						if (S_ISDIR(sb.st_mode)) {
							FBTEST_MSG_DEBUG("Path is a directory Path\n");
							path = DIRPATH;
						} else if (S_ISREG(sb.st_mode)) {
							FBTEST_MSG_DEBUG("Path is a regular file Path\n");
							path = REGFILE;
						} else {
							FBTEST_MSG_DEBUG("Path is not of a regular file or directory\n");
							path = OTHER;
							return 1;
						}
					} else {
						FBTEST_MSG_DEBUG("Input Path is not a valid path. (%s)\n", thisFBTEST->path);
						showError(-INVALID_PATH, 1);
					}
				} else {
					FBTEST_MSG_DEBUG("file name is null\n");
					showError(-FILE_NOT_FOUND, 1);
				}
				break;
			case 'D':
                                snprintf(thisFBTEST->dumpPath, sizeof(thisFBTEST->dumpPath), "%s", optarg);
				break;
			case 'h':
				thisFBTEST->outResolution.height = atoi(optarg);
				break;
			case 'w':
				thisFBTEST->outResolution.width = atoi(optarg);
				break;
			case 'S':
				thisFBTEST->isStepScale = atoi(optarg);
				break;
			case 'd':
				if (!(strncasecmp(optarg, "0", sizeof(optarg))))
					thisFBTEST->rotateDegree =  ROT_0;
				else if (!(strncasecmp(optarg, "90", sizeof(optarg))))
					thisFBTEST->rotateDegree =  ROT_90;
				else if (!(strncasecmp(optarg, "180", sizeof(optarg))))
					thisFBTEST->rotateDegree =  ROT_180;
				else if (!(strncasecmp(optarg, "270", sizeof(optarg))))
					thisFBTEST->rotateDegree =  ROT_270;
				else
					showUsage();
				break;
			case 'f':
				if (!(strncasecmp(optarg, "NOFLIP", sizeof(optarg))\
					&& strncmp(optarg, "0", sizeof(optarg))))
					thisFBTEST->rotateFlip =  FLIP_NOP;
				else if (!(strncasecmp(optarg, "LR", sizeof(optarg))))
					thisFBTEST->rotateFlip =  FLIP_LR;
				else if (!(strncasecmp(optarg, "UD", sizeof(optarg))))
					thisFBTEST->rotateFlip =  FLIP_UD;
				else if (!(strncasecmp(optarg, "LRUD", sizeof(optarg)) &&\
					strncasecmp(optarg, "UDLR", sizeof(optarg))))
					thisFBTEST->rotateFlip =  FLIP_LRUD;
				else
					showUsage();
				break;
			case 'c':
				if (!(strcasecmp(optarg, "25")))
					thisFBTEST->crop |= 1<<P_25;
				else if (!(strcasecmp(optarg, "50")))
					thisFBTEST->crop |= 1<<P_50;
				else if (!(strcasecmp(optarg, "75")))
					thisFBTEST->crop |= 1<<P_75;
				else if (!(strcasecmp(optarg, "BOTTOM")))
					thisFBTEST->crop |= 1<<C_BOTTOM;
				else if (!(strcasecmp(optarg, "TOP")))
					thisFBTEST->crop |= 1<<C_TOP;
				else if (!(strcasecmp(optarg, "LEFT")))
					thisFBTEST->crop |= 1<<C_LEFT;
				else if (!(strcasecmp(optarg, "RIGHT")))
					thisFBTEST->crop |= 1<<C_RIGHT;
				else if (!(strcasecmp(optarg, "RAND")))
					thisFBTEST->crop |= 1<<C_RAND;
				else
					showUsage();
				break;
			case 'H':
				thisFBTEST->cropresolution.height = atoi(optarg);
				break;
			case 'W':
				thisFBTEST->cropresolution.width  = atoi(optarg);
				break;
			case 'i':
				thisFBTEST->coordinate.x = atoi(optarg);
				break;
			case 'j':
				thisFBTEST->coordinate.y = atoi(optarg);
				break;
			case 'x':
				thisFBTEST->imgOffset.startX = atoi(optarg);
				break;
			case 'y':
				thisFBTEST->imgOffset.startY = atoi(optarg);
				break;
			case 'X':
				thisFBTEST->imgOffset.endX = atoi(optarg);
				break;
			case 'Y':
				thisFBTEST->imgOffset.endY = atoi(optarg);
				break;
			case 'F':
				thisFBTEST->fps = atoi(optarg);
				break;
			case '1':
                                snprintf(thisFBTEST->blend.ImagePath[0], sizeof(thisFBTEST->blend.ImagePath[0]), "%s", optarg);
				break;
			case '2':
                                snprintf(thisFBTEST->blend.ImagePath[1], sizeof(thisFBTEST->blend.ImagePath[1]), "%s", optarg);
				break;
			case '3':
                                snprintf(thisFBTEST->blend.ImagePath[2], sizeof(thisFBTEST->blend.ImagePath[2]), "%s", optarg);
				break;
			case '4':
                                snprintf(thisFBTEST->blend.ImagePath[3], sizeof(thisFBTEST->blend.ImagePath[3]), "%s", optarg);
				break;
			case 'P':
				thisFBTEST->blend.ImgAlphaVal[0] = atoi(optarg);
				break;
			case 'Q':
				thisFBTEST->blend.ImgAlphaVal[1] = atoi(optarg);
				break;
			case 'R':
				thisFBTEST->blend.ImgAlphaVal[2] = atoi(optarg);
				break;
			case 'U':
				thisFBTEST->blend.ImgAlphaVal[3] = atoi(optarg);
				break;
			case 'T':
				if (!strncasecmp(optarg, "0x", 2))
					thisFBTEST->transparency = (unsigned int)strtoll(optarg, NULL, 16);
				else
					thisFBTEST->transparency = (unsigned int)strtoll(optarg, NULL, 10);
				break;
			case 'I':
				thisFBTEST->iteration = atoi(optarg);
				break;
			case 'B':
				snprintf(thisFBTEST->pp_data.postproc_file, strlen(optarg) + 1, "%s", optarg);
				thisFBTEST->pp_data.postproc_use_file = 1;
				break;
			case ':':
				fprintf(stderr, "\nError - Option `%c' needs a value\n\n", optopt);
				showUsage();
				break;
			case '?':
				fprintf(stderr, "\nError - No such option: `%c'\n\n", optopt);
				showUsage();
				break;
			case 'k':
				interactive = TRUE;
				interactiveSleep = atoi(optarg);
				break;
			case 'O':
				printf("GOT -O %s\n", optarg);
				if (!(strncasecmp(optarg, "USR", sizeof(optarg)) &&
					strncmp(optarg, "0", sizeof(optarg))))
					thisFBTEST->moveType =  USER_DEFINED;
				else if (!(strncasecmp(optarg, "HORIZONTAL", sizeof(optarg)) &&
					strncmp(optarg, "1", sizeof(optarg))))
					thisFBTEST->moveType =  HORIZONTAL;
				else if (!(strncasecmp(optarg, "VERTICAL", sizeof(optarg)) &&
					strncmp(optarg, "2", sizeof(optarg))))
					thisFBTEST->moveType =  VERTICAL;
				else if (!(strncasecmp(optarg, "BOUNDARY", sizeof(optarg)) &&
					strncmp(optarg, "3", sizeof(optarg))))
					thisFBTEST->moveType =  BOUNDARY;
				else if (!(strncasecmp(optarg, "DIAGONALS", sizeof(optarg)) &&
					strncmp(optarg, "4", sizeof(optarg))))
					thisFBTEST->moveType =  DIAGONALS;
				else if (!(strncasecmp(optarg, "RANDOM", sizeof(optarg)) &&
					strncmp(optarg, "5", sizeof(optarg))))
					thisFBTEST->moveType =  RANDOM;
				else
					showUsage();
				break;
#ifdef MDP3_FLAG
			case 'A':
				printf("************ASYNC*************************\n");
				async = 1;
				printf("Aync flag is set to %d \n",async);
				break;
			case 'Z':
				printf("dump set");
				dump = 1;
				break;
#endif
			case 'e':
				snprintf(thisFBTEST->crcPath, sizeof(thisFBTEST->crcPath), "%s", optarg);
				break;

			default :
				fprintf(stderr, "Invalid argument: %c\n", opt);
				showUsage();
		}

		if (shiftCountByName != -1) {
			if (testName) {
				int key = lookup_key_case_insensitive(testNameLUT, testName, -1);
				int val = lookup_val(testNameLUT, key, -1);
				testEnable |= 1 << val;
			}
		} else if(shiftCountByNum != -1) {
			testEnable |= 1 << shiftCountByNum;
		}
	}

    /*
    // print all remaining options
    */
    for(; optind < argc; optind++) {

		if ((!(strcasecmp(argv[optind], "on")))) {
			deInterlaceFlag = ENABLED;
			ditherFlag = ENABLED;
		}

		if ((!(strcasecmp(argv[optind], "bg"))))
			colorkeyflag = 1;//for BG color keying

		if ((!(strcasecmp(argv[optind], "fg"))))
			colorkeyflag = 0;//for FG color keying, default color keying is FG

		if ((!(strcasecmp(argv[optind], "mcs"))))
			multiopflag = 0;//for multi-op test (move-crop-scale),this is default

		if ((!(strcasecmp(argv[optind], "rmcs"))))
			multiopflag = 1;//for multi-op test (rotate-move-crop-scale)

		if ((!(strcasecmp(argv[optind], "const"))))
			thisFBTEST->blend.isPixBlend = 0;

		if ((!(strcasecmp(argv[optind], "pix"))))
			thisFBTEST->blend.isPixBlend = 1;

		if ((!(strcasecmp(argv[optind], "pm"))))
			thisFBTEST->blend.ispremul = 1;	/* for pre multiplying */

		if ((!(strcasecmp(argv[optind], "crc0")))) {
			thisFBTEST->crcmode = 0;
			printf("GOT CRC MODE 0");
		} else if ((!(strcasecmp(argv[optind], "crc1")))) {
			thisFBTEST->crcmode = 1;
			printf("GOT CRC MODE 1");
		}
		if ((!(strcasecmp(argv[optind], "misr")))) {
			printf("GOT MISR");
			thisFBTEST->misr = 1;
		}
		if ((!(strcasecmp(argv[optind], "hist")))) {
			printf("GOT HISTOGRAM");
			thisFBTEST->misr = 2;
		}
		if ((!(strcasecmp(argv[optind], "decimation")))) {
			printf("Enable decimation\n");
			thisFBTEST->decimation = TRUE;
		}
    }
	if (testEnable == 0) {
		printf("Nothing to test. Please enter valid test name or test number\n");
		showUsage();
	}
	return ret;
}

void reset_fbtest()
{
	struct fbtest_params *thisFBTEST;
	testEnable = 0;

	thisFBTEST = &FBTEST;
	FB = &(fb[FB_NUM_DEFAULT]);
	path = -1;
	verbose = DISABLED;
	curr_overlay_id[0] = MSMFB_NEW_REQUEST;
	curr_overlay_id[1] = MSMFB_NEW_REQUEST;
	curr_overlay_id[2] = MSMFB_NEW_REQUEST;
	curr_overlay_id[3] = MSMFB_NEW_REQUEST;

	memset(thisFBTEST, 0x00, sizeof(FBTEST));

	thisFBTEST->testMode = AUTO;
	thisFBTEST->rotateDegree = DEFAULT_ROTATE_FLIP;
	thisFBTEST->rotateFlip = DEFAULT_ROTATE_FLIP;
	thisFBTEST->blend.ImgAlphaVal[0] = -1;
	thisFBTEST->blend.ImgAlphaVal[1] = -1;
	thisFBTEST->blend.ImgAlphaVal[2] = -1;
	thisFBTEST->blend.ImgAlphaVal[3] = -1;
}

int checkTestExist(char const* lookupTable[], unsigned int size, int testEnable){
	unsigned int i = 0;
	int shiftCountByName = -1;
	for (i = 0; i < size; ++i) {
		shiftCountByName = lookup_key_case_insensitive(testNameLUT, lookupTable[i], -1);
		if ((shiftCountByName != -1) && (testEnable & (1 << shiftCountByName)))
			return TRUE;
	}
	return FALSE;
}

static void print_user_args(struct fbtest_params *thisFBTEST)
{
	VPRINT(verbose, "\n\t\t\t USER INPUT ARGUMENTS\n");
	VPRINT(verbose, "##################################################\n");
	VPRINT(verbose, "Frame Buffer:           %s\n",
			(thisFBTEST->fBuffer == FB0) ? "fb0" :
			(thisFBTEST->fBuffer == FB1) ? "fb1" :
			(thisFBTEST->fBuffer == FB2) ? "fb2" : "fb01");
	VPRINT(verbose, "File/Directory Path:   %s\n", thisFBTEST->path);
	VPRINT(verbose, "fbtest Mode:		%s\n",
			thisFBTEST->testMode == USER ? "user" :
			thisFBTEST->testMode == AUTO ? "auto" : "negative");
	VPRINT(verbose, "fbtest case:           %s\n",
		(lookup_string(testNameLUT, ffs(testEnable) - 1, "Unknown")));
	VPRINT(verbose, "Output WxH:            %dx%d\n",
			thisFBTEST->outResolution.width,
			thisFBTEST->outResolution.height);
	VPRINT(verbose, "Scale in Step :        %s\n",
			thisFBTEST->isStepScale ? "yes" : "no");
	VPRINT(verbose, "Rotate Degree:         %s\n",
			(thisFBTEST->rotateDegree == ROT_0) ? "0" :
			(thisFBTEST->rotateDegree == ROT_90) ? "90" :
			(thisFBTEST->rotateDegree == ROT_180) ? "180" : "270");
	VPRINT(verbose, "Flip Mode:             %s\n",
			(thisFBTEST->rotateFlip == FLIP_NOP) ? "none" :
			(thisFBTEST->rotateFlip == FLIP_LR) ? "LR" :
			(thisFBTEST->rotateFlip == FLIP_UD) ? "UD" : "LRUD");
	VPRINT(verbose, "Crop Mode:             %s\n",
			(thisFBTEST->crop & (1 << P_25)) ? "25%" :
			(thisFBTEST->crop & (1 << P_50)) ? "50%" :
			(thisFBTEST->crop & (1 << P_75)) ? "75%" :
			(thisFBTEST->crop & (1 << C_BOTTOM)) ? "bottom half" :
			(thisFBTEST->crop & (1 << C_TOP)) ? "top half" :
			(thisFBTEST->crop & (1 << C_LEFT)) ? "left half" :
			(thisFBTEST->crop & (1 << C_RIGHT)) ? "right half" :
			(thisFBTEST->crop & (1 << C_RAND)) ? "random" : "none");
	VPRINT(verbose, "Crop WxH:              %dx%d\n",
			thisFBTEST->cropresolution.width,
			thisFBTEST->cropresolution.height);
	VPRINT(verbose, "Crop x,y coordinate:   %d,%d\n",
			thisFBTEST->coordinate.x, thisFBTEST->coordinate.y);
	VPRINT(verbose, "Move offsets:          [%d,%d]-->[%d,%d]\n",
			thisFBTEST->imgOffset.startX, thisFBTEST->imgOffset.startY,
			thisFBTEST->imgOffset.endX, thisFBTEST->imgOffset.endY);
	VPRINT(verbose, "Move Type:             %s\n",
			(thisFBTEST->moveType == USER_DEFINED) ? "USER_DEFINED" :
			(thisFBTEST->moveType == HORIZONTAL) ? "HORIZONTAL" :
			(thisFBTEST->moveType == VERTICAL) ? "VERTICAL" :
			(thisFBTEST->moveType == BOUNDARY) ? "BOUNDARY" :
			(thisFBTEST->moveType == DIAGONALS) ? "DIAGONALS" : "RANDOM");
	VPRINT(verbose, "Video fps:             %d\n", thisFBTEST->fps);
	VPRINT(verbose, "Input images:          %s, %s, %s, %s\n",
			(*thisFBTEST->blend.ImagePath[0]) ?
			thisFBTEST->blend.ImagePath[0] : "none",
			(*thisFBTEST->blend.ImagePath[0]) ?
			thisFBTEST->blend.ImagePath[1] : "none",
			(*thisFBTEST->blend.ImagePath[0]) ?
			thisFBTEST->blend.ImagePath[2] : "none",
			(*thisFBTEST->blend.ImagePath[0]) ?
			thisFBTEST->blend.ImagePath[3] : "none");
	VPRINT(verbose, "Alpha values:          %d, %d, %d\n",
			thisFBTEST->blend.ImgAlphaVal[0],
			thisFBTEST->blend.ImgAlphaVal[1],
			thisFBTEST->blend.ImgAlphaVal[2]);
	VPRINT(verbose, "Transperancy value:    %d\n",
			thisFBTEST->transparency);
	VPRINT(verbose, "##################################################\n");
}
#ifdef QPIC_FLAG
#include "yellow_face.zif"
static int render_buffer(int x, int y, int w, int h)
{
	int result = 0;
	unsigned char *pTemp = (char *)FB->fb_buf;
	struct fb_var_screeninfo *vinfo = &(FB->fb_vinfo);

	int right = x + w;
	int bottom = y + h;
	if (right > vinfo->xres)
		right = vinfo->xres;
	if (bottom > vinfo->yres)
		bottom = vinfo->yres;

	int i, j;

	x = 0;
	y = 0;
	right = vinfo->yres;
	bottom = vinfo->yres;
	for(i=y; i< bottom; i++)
	{
		for(j=x; j<right; j++)
		{
			unsigned short data = yellow_face_data[(((i-y)  % 128) * 128) + ((j-x) %128)];
			pTemp[i*FB->fb_finfo.line_length + (j*4) + 2] = (unsigned char)((data & 0xF800) >> 11 << 3);
			pTemp[i*FB->fb_finfo.line_length + (j*4) + 1] = (unsigned char)((data & 0x7E0) >> 5 << 2);
			pTemp[i*FB->fb_finfo.line_length + (j*4) + 0] = (unsigned char)((data & 0x1F) << 3);
		}
	}
	result = ioctl(FB->fb_fd, FBIOPAN_DISPLAY, vinfo);
	if (result < 0)
		printf("\n ERROR: FBIOPAN_DISPLAY failed! line=%d\n",__LINE__);
	return result;
}

int main(int argc, char **argv) {
	int result = TEST_RESULT_FAIL;
	int x,y,w,h;

	FB =  &(fb[0]);
	strlcpy(FB->fb_name, FB_NAME_STR_LE, sizeof(FB->fb_name));
	FB->fb_fd = open(FB->fb_name, O_RDWR | O_SYNC, 0);
	if (FB->fb_fd < 0)
		return -FBDEV_OPEN_FAILED;
	else
		FBTEST_MSG_DEBUG("[%s(%d)]: %s opened FB successfully!\n", __func__ , __LINE__, FB->fb_name);

	result = getFBInfo(FB);

	if (getPanelInfo(FB))
		FBTEST_MSG_DEBUG("WARNING: Cannot obtain Panel Type.\n");

	result = mapFB(FB);

	x=0;
	y=0;
	w=0;
	h=0;
	if (argc >1)
		x = atoi(argv[1]);
	if (argc >2)
		y = atoi(argv[2]);
	if (argc >3)
		w = atoi(argv[3]);
	if (argc >4)
		h = atoi(argv[4]);

	FBTEST_MSG_DEBUG(" mapFB result = %d, offset = %x x=%d, y=%d, w=%d, h=%d \n", result, (int)FB->fb_buf, x, y, w, h);

	result = render_buffer(x, y, w, h);
	usleep(TEST_DELAY_LONG);

	if (FB->fb_fd >= 0) {
		//unmap
		if ((FB->fb_buf != NULL) && (FB->fb_buf != MAP_FAILED)) {
			munmap(FB->fb_buf,FB->fb_finfo.smem_len);
		}
		close(FB->fb_fd);
	}

	return result;
}
#else
//UTF: Below functions added for fbtest enhancement.
int main(int argc, char **argv) {

	unsigned long long i;
	uint8_t fbcount;
	int result = TEST_RESULT_FAIL;
	int testResult = 0; // Default = No Error
	int testResultTotal = 0, testResultPass = 0, testResultSkip = 0, testResultFail = 0;
	DIR  *dip;
	struct dirent   *dit;
	struct fbtest_params *thisFBTEST;
	struct inputFileParams thisInputFileParams;
	struct msmfb_metadata metadata;
	// Check if the user gave a config file
	int num_line = 1;
	int using_config_file = 0;

	char* argv_array[100][MAX_FILE_PATH];
	int argc_array[100];
	int config_loop_counter;
	int shiftCountByName = -1;
	int ret = -1;
	const char* userModeTest1[] = {"fpsvsync", "adversarial", "overlayScale", "overlayStress", "allcolorFormatOverlay", "overlayARGB", "postproc", "overlay", "overlayAlpha", "postproc"};
	const char* userModeTest2[] = {"fpsvsync", "adversarial", "overlayScale", "overlayStress", "allcolorFormatOverlay", "overlayARGB", "postproc", "overlay", "overlayAlpha", "blend"};
	int numOfUserModeTest1 = 10;
	int numOfUserModeTest2 = 10;
	char crcpathReport[256];
	FILE *report;
	if (argc != 1) {
		if (argv[1][0] == '-' && argv[1][1] == 'C') {
			if (argc == 3) {
				parse_config_file(argv[2], &num_line, argv_array, argc_array);
				using_config_file = 1;
			} else {
				printf("Please enter config file name with -C option.\n");
				exit(0);
			}
        }
	}else{
		showUsage();
	}

	reset_fbtest();

	// MDP
	mdp_version = MDP_V_UNKNOWN;
	panel_type = PANEL_UNKNOWN;
	thisFBTEST = &FBTEST;
	result = chooseMEMDev();
	if (result < 0)
		showError(result, 1);

	result = openMEMDev();
	if (result < 0)
		showError(result, 1);

	// Register signal and signal handler
	signal(SIGINT, signal_callback_handler); 	//UTF: added for cleanup code addition.

#ifdef MDP3_FLAG
	memcpy(testFunctions, MDP3testFunctions, MDP3TestFuncArraySize);
	memcpy(testNameLUT, MDP3testNameLUT, MDP3TestNameArraySize);
#endif

#ifdef MDP4_FLAG
	memcpy(testFunctions, MDP4testFunctions, MDP4TestFuncArraySize);
	memcpy(testNameLUT, MDP4testNameLUT, MDP4TestNameArraySize);
#endif

	if((!strcmp((char*)testFunctions, "")) || (!strcmp((char*) testNameLUT, ""))){
		printf("\nfbtest does not support this TARGET. Please check Android.mk of fbtest.\n\n");
		exit(0);
	}

	/*
	   For each line in the config file, parse options and run tests
	 */
	for (config_loop_counter = 0; config_loop_counter < num_line; config_loop_counter++) {
		// parse commandline options
		if (using_config_file) {
			VPRINT(verbose, "Executing Config File Line %d\n", config_loop_counter + 1);
			if (parse_opt(argc_array[config_loop_counter], argv_array[config_loop_counter])) {
				showUsage();
				return -1;
			}
		}
		else {
			if (parse_opt(argc, argv)) {
				showUsage();
				return -1;
			}
		}
		print_user_args(thisFBTEST);

		result = chooseFBDev(FB_NUM_DEFAULT);
		if (result < 0)
			showError(result, 1);

		for (fbcount = 0; fbcount < FBLISTSIZE_GLOBAL; fbcount++) {
			result = openFBDev(FBLIST_GLOBAL[fbcount]);
			if (result < 0)
				showError(result, 1);

			result = getFBInfo(FBLIST_GLOBAL[fbcount]);
			if (result < 0)
				showError(result, 1);

			if (getMDPInfo(FBLIST_GLOBAL[fbcount]) == MDP_V_UNKNOWN) {
				FBTEST_MSG_DEBUG("WARNING: Cannot obtain MDP Hardware Version. Trying with Default (MDP 4.0)\n");
				mdp_version = MDP_V4_0;
			}

			if (mdp_version < MDP_V3_0) {
				printf("This test app currently supports MDP3 and above versions only\n");
				mdp_version = MDP_V3_0;
			}

			if (mdp_version >= MDP_V4_0) {
				int en = 1;
				if (ioctl(FBLIST_GLOBAL[fbcount]->fb_fd, MSMFB_OVERLAY_PLAY_ENABLE, &en) < 0)
					FBTEST_MSG_DEBUG("WARNING: Cannot enable OV PLAY.\n");
			}

			if (getPanelInfo(FBLIST_GLOBAL[fbcount]))
				FBTEST_MSG_DEBUG("WARNING: Cannot obtain Panel Type.\n");

			chooseFBFmt(0, FBLIST_GLOBAL[fbcount], 1);		// Auto-select FB format based on FB number
			if (thisFBTEST->fBuffer == FB0) {
				result = mapFB(FBLIST_GLOBAL[fbcount]);
				if (result < 0)
					showError(result, 1);
			}
			FBTEST_MSG_DEBUG("FBLIST_GLOBAL[count]->fb_num fb%d\n", FBLIST_GLOBAL[fbcount]->fb_num);

			metadata.op = metadata_op_base_blend;
			metadata.flags = 0;
			metadata.data.blend_cfg.is_premultiplied = 0;
			// Restricting this below MDP_V5 as this ioctl is not supported in mdp5.
			if (mdp_version < MDP_V5 ) {
				if (ioctl(FBLIST_GLOBAL[fbcount]->fb_fd, MSMFB_METADATA_SET, &metadata) == -1) {
					FBTEST_MSG_DEBUG("failed to configure alpha mode \n");
				}
			}
		}

		if(thisFBTEST->crcPath)
			strlcpy((char *)&crcpathReport, thisFBTEST->crcPath, sizeof(crcpathReport));
		strlcat(crcpathReport, ".report", sizeof(crcpathReport));
		printf("\n\n Report file name (%s)", crcpathReport);

		if (thisFBTEST->crcmode) {
			report = fopen(crcpathReport, "a");
			if (report == NULL) {
				printf("Cant open file %s @ line %d\n", crcpathReport, __LINE__);
				return TEST_RESULT_FAIL;
			} else {
				fprintf(report, "\n/****************************/\n");
				fprintf(report, "/***** CRC Report Start *****/\n");
				fprintf(report, "/****************************/\n");
				fclose(report);
			}
		}

		// Print test and framebuffer info
		// Startup
		VPRINT(verbose, "\t\t\tFBTEST SETUP INFORMATION\n");
		VPRINT(verbose, "----------------------------------------------------------------------\n");
		VPRINT(verbose, "Test Mode = %s\n",thisFBTEST->testMode==0 ? "USER" : "AUTO");
		VPRINT(verbose, "Memory Allocation dev: %s\n",
				(MEM ? MEM->mem_name : "None"));
		VPRINT(verbose, "\nFramebuffer Info:\n");
		VPRINT(verbose, "-------------------------\n");
		VPRINT(verbose, " Framebuffer Device = fb%d\n", FB->fb_num);
		VPRINT(verbose, " Framebuffer Color Format = %s (%d)\n",
				lookup_string(format_name_table,FB->fb_fmt, "unknown"),
				FB->fb_fmt);
		VPRINT(verbose, " X Resolution (visible) = %d\n", FB->fb_vinfo.xres);
		VPRINT(verbose, " Y Resolution (visible) = %d\n", FB->fb_vinfo.yres);
		VPRINT(verbose, " X Resolution (virtual) = %d\n", FB->fb_vinfo.xres_virtual);
		VPRINT(verbose, " Y Resolution (virtual) = %d\n", FB->fb_vinfo.yres_virtual);
		VPRINT(verbose, " Pixel Depth = %d bpp\n", FB->fb_vinfo.bits_per_pixel);
		VPRINT(verbose, " Pixel Clock = %d Hz\n", FB->fb_vinfo.pixclock);
		VPRINT(verbose, " Framebuffer ID = %s\n", FB->fb_finfo.id);
		VPRINT(verbose, " Line Length = %d\n", FB->fb_finfo.line_length);
		VPRINT(verbose, " Hardware Version ID = MDP %.2f\n",
				mdp_version / 100.0);
		VPRINT(verbose, " Panel Type = %s\n",
				((panel_type == PANEL_PRIMARY_LCD) ? "Primary LCD" :
				(panel_type == PANEL_SECONDARY_LCD) ? "Secondary LCD" :
				((panel_type == PANEL_TVOUT) ? "TV OUT" : "Unknown")));
		VPRINT(verbose, " Framebuffer SMEM Start = 0x%X\n",
				(unsigned int)FB->fb_finfo.smem_start);
		VPRINT(verbose, " Framebuffer SMEM Length = %d bytes\n", FB->fb_finfo.smem_len);
		VPRINT(verbose, " Framebuffer Active Buffer = 0x%p\n", FB->fb_buf);
		VPRINT(verbose, " Framebuffer Back Buffer   = 0x%p\n", FB->fb_buf_back);
		VPRINT(verbose, "----------------------------------------------------------------------\n");

		VPRINT(verbose, "\n\t\t\t======STARTING FBTEST======\n");

                {//MDP4 & MDP5 & MDP3
			if(checkTestExist(userModeTest1, ARRAY_SIZE(userModeTest1), testEnable)){
				thisFBTEST->testMode = USER;
			}

			shiftCountByName = lookup_key_case_insensitive(testNameLUT, "postproc", -1);
			if (shiftCountByName != -1) {
				if ((testEnable & (1 << shiftCountByName))) {
					if (strlen(thisFBTEST->path) == 0) {
						strlcpy(thisFBTEST->path, "/dummy_176x144_RGB_888.rgb", sizeof(thisFBTEST->path));
					}
				}
			}

			if (thisFBTEST->testMode == USER) {
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "postproc", -1);
				if ((shiftCountByName != -1) && (testEnable & (1 << shiftCountByName))) {
						path = REGFILE;
				} else if (strcmp(thisFBTEST->path, "") == 0) {
					path = REGFILE;
					strlcpy(thisFBTEST->path, "/dummy_176x144_RGB_888.rgb", sizeof(thisFBTEST->path));
				} else if (checkTestExist(userModeTest2, numOfUserModeTest2, testEnable)){
						path = REGFILE;
					strlcpy(thisFBTEST->path, "/dummy_176x144_RGB_888.rgb", sizeof(thisFBTEST->path));
				} else {
					VPRINT(verbose, "\nInput Image: %s\n", thisFBTEST->path);
					path = REGFILE;
				}

				if ((path != REGFILE)) {
					FBTEST_MSG_DEBUG("Can't run in user mode without regular file Path\n");
					showError(-INVALID_PATH, 1);
				}

				char *ptr =strrchr(thisFBTEST->path,'.');

				if (ptr) {
					if ((!strcmp(ptr, ".rgb")) || (!strcmp(ptr, ".yuv"))) {
						result = parseFileName(thisFBTEST->path, &thisInputFileParams);
						if (result < 0) {
							testResultSkip++;
							showError(result, 1);
						}else{
							//File Name parsing sucess
							strlcpy(thisFBTEST->fileParams.filenamePath, thisInputFileParams.filenamePath, sizeof(thisInputFileParams.filenamePath));
							strlcpy(thisFBTEST->fileParams.charFormat, thisInputFileParams.charFormat, sizeof(thisInputFileParams.charFormat));
							strlcpy(thisFBTEST->fileParams.filename, thisInputFileParams.filename, sizeof(thisInputFileParams.filename));

							thisFBTEST->fileParams.inputFormat = thisInputFileParams.inputFormat;
							thisFBTEST->fileParams.inResolution.width = thisInputFileParams.inResolution.width;
							thisFBTEST->fileParams.inResolution.height = thisInputFileParams.inResolution.height;
						}
					}else{
						FBTEST_MSG_DEBUG("Input file is not an rgb or yuv file\n");
						showError(-PARSE_FAILED, 1);
					}
				}else{
					FBTEST_MSG_DEBUG("File name doesn't contain extension\n");
					showError(-PARSE_FAILED, 1);
				}

				// Run User tests
				for (i = 0; i < ARRAY_SIZE(testFunctions); i++) {
					if (testEnable & (1U << i)) {
						result = testFunctions[i]();
						if (result == TEST_RESULT_PASS)
							testResultPass++;
						else if (result == TEST_RESULT_SKIP)
							testResultSkip++;
						else{
							testResultFail++;
							showError(result, 0);
						}
						testResultTotal++;
					}
				}
			} else if (thisFBTEST->testMode == AUTO) {
				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "fpsvsync", -1);
				if (shiftCountByName != -1) {
					if ((testEnable & (1 << shiftCountByName))) {
						path = DIRPATH;
						strlcpy(thisFBTEST->path, ".", sizeof(thisFBTEST->path));
					}
				}

				shiftCountByName = lookup_key_case_insensitive(testNameLUT, "postproc", -1);
				if (shiftCountByName != -1) {
					if ((testEnable & (1 << shiftCountByName))) {
						path = DIRPATH;
						strlcpy(thisFBTEST->path, ".", sizeof(thisFBTEST->path));
					}
				}

				if ((path != DIRPATH)) {
					FBTEST_MSG_DEBUG("Can't run in auto mode without directory path\n");
					showError(-INVALID_PATH, 1);
				}

				if ((dip = opendir(thisFBTEST->path)) == NULL) {
					FBTEST_MSG_DEBUG("Failed to open the directory\n");
					showError(-DIROPEN_FAILED, 1);
				}

				FBTEST_MSG_DEBUG("Input directory opened successfuly\n");

				while((dit = readdir(dip)) != NULL) {
					char *ptr =strrchr(dit->d_name,'.');
					if (ptr) {
						if ((!strcmp(ptr, ".rgb")) || (!strcmp(ptr, ".yuv"))) {
							char *tmp = malloc(strlen(thisFBTEST->path) + 1);
							if (!tmp)
								return -MALLOC_FAILED;
							memset(tmp,0x00,(strlen(thisFBTEST->path) + 1));
							strcat(tmp,thisFBTEST->path);
							strcat(tmp,"/");
							strcat(tmp,dit->d_name);
							VPRINT(verbose, "\nInput Image: %s\n", dit->d_name);
							FBTEST_MSG_DEBUG("Parsing file name to get format & resolution\n");
							result =parseFileName(tmp, &thisInputFileParams);
							if (result >= 0) {
								strlcpy(thisFBTEST->fileParams.filenamePath, thisInputFileParams.filenamePath, sizeof(thisFBTEST->fileParams.filenamePath));
								strlcpy(thisFBTEST->fileParams.charFormat, thisInputFileParams.charFormat, sizeof(thisFBTEST->fileParams.charFormat));

								thisFBTEST->fileParams.inputFormat = thisInputFileParams.inputFormat;
								thisFBTEST->fileParams.inResolution.width = thisInputFileParams.inResolution.width;
								thisFBTEST->fileParams.inResolution.height = thisInputFileParams.inResolution.height;

								// Run Auto tests
								for (i = 0; i < ARRAY_SIZE(testFunctions); i++) {
									if (testEnable & (1U << i)) {
										result = testFunctions[i]();
										if (result == TEST_RESULT_PASS)
											testResultPass++;
										else if (result == TEST_RESULT_SKIP)
											testResultSkip++;
										else{
											testResultFail++;
											showError(result, 0);
										}
										testResultTotal++;
									}
								}
							}else{
								testResultSkip++;
								showError(result, 0);
								testResultTotal++;
							}
						}
					}
				}

				if (closedir(dip) == -1) {
					FBTEST_MSG_DEBUG("Error in closing Direcotory\n");
					showError(-DIRCLOSE_FAILED, 1);
				}
				FBTEST_MSG_DEBUG("Input directory closed\n");

			}else{
				FBTEST_MSG_DEBUG("Invalid Mode\n");
				showError(-INVALID_PARAMS, 1);
			}
		}
		if (thisFBTEST->crcmode) {
			report = fopen(crcpathReport, "a");
			if (report == NULL) {
				printf("Cant open file %s @ line %d\n", crcpathReport, __LINE__);
				return TEST_RESULT_FAIL;
			} else {
				fprintf(report, "\n/****************************/\n");
				fprintf(report, "/****** CRC Report End ******/\n");
				fprintf(report, "/****************************/\n");
				fclose(report);
			}
		}
		printf("\n  Done: Total %d tests.\n", testResultTotal);
		printf("%d (%d %c) PASS, %d (%d %c) SKIPPED, %d (%d %c) FAIL.\n\n",\
				testResultPass, (testResultPass*100)/testResultTotal,'%',\
				testResultSkip, (testResultSkip*100)/testResultTotal,'%',\
				testResultFail, (testResultFail*100)/testResultTotal,'%');

		metadata.op = metadata_op_base_blend;
		metadata.flags = 0;
		metadata.data.blend_cfg.is_premultiplied = 1;

		// Restricting this below MDP_V5 as this ioctl is not supported in mdp5.
		if (mdp_version < MDP_V5 ) {
			if (ioctl(FB->fb_fd, MSMFB_METADATA_SET, &metadata) == -1)
				FBTEST_MSG_DEBUG("failed to configure alpha mode \n");
		}

		cleanup_overlay(); 	//UTF: added for cleanup code addition.

		testResult += testResultFail;

		testResultTotal = 0;
		testResultPass = 0;
		testResultSkip = 0;
		testResultFail = 0;
		reset_fbtest();
	}
	close_devices(); 	//UTF: added for cleanup code addition.

	if (testResult > 0)
		return TEST_RESULT_FAIL;
	else
		return TEST_RESULT_PASS;
}
#endif
