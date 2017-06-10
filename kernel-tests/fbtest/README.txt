
=======================================================
framebuffer Enhanced test program
=======================================================

This directory contains Linux framebuffer test program.

It requires msm_fb framebuffer (/dev/fb) driver and msm_rotator driver
(/dev/msm_rotator) which provides the standard and extended framebuffer APIs.
-------------------------------------------------------------------------------

Framebuffer test program enhancement (fbtest v1.0)
================================================================

Naming Convention for input test image file.
--------------------------------------------

	Note:- Name of input image test file should be in specified format(case insensitive):-filename_resolution_format.rgb/.yuv

		   example:- filename1_320x240_RGB_565.rgb
			     filename2_288x352_Y_CBCR_H2V2.yuv

		   format portion(case insensitive) in file name can be one of these:-

		   RGB_565,XRGB_8888,Y_CBCR_H2V2,ARGB_8888,RGB_888,Y_CRCB_H2V2,YCRYCB_H2V1,Y_CRCB_H2V1,Y_CBCR_H2V1,RGBA_8888,BGRA_8888,
		   RGBX_8888,Y_CRCB_H2V2_TILE,Y_CBCR_H2V2_TILE,Y_CR_CB_H2V2,Y_CB_CR_H2V2,Y_CRCB_H1V1,Y_CBCR_H1V1,BGR_565,Y_CR_CB_GH2V2,
		   RGBA_8888_TILE, ARGB_8888_TILE, ABGR_8888_TILE, BGRA_8888_TILE, RGBX_8888_TILE, XRGB_8888_TILE, XBGR_8888_TILE, BGRX_8888_TILE
		   Y_CBCR_H2V2_ADRENO, CBYCRY_H2V1, Y_CRCB_H1V2, Y_CBCR_H1V2, YCRCB_H1V1, YCBCR_H1V1, BGR_888, Y_CBCR_H2V2_VENUS, BGRX_8888, YCBYCR_H2V1.

Parameter list in new fbtest
----------------------------

To run new fbtest:

	# ./fbtest [arguments]

	./fbtest 	-u ---usage
			-V ---version
			-v ---verbose

			-m (user/auto) ---default is auto
			-p (Input file/directory path) ---mandatory for most of the MDP4 tests. See examples.
			-D (mixer-2 ouput dump directory path) ---mandatory for all fb2/mixer-2 tests. See examples.
			-t (format/scale/rotate/all/ ...) ---mandatory argument
			-b (fb0/fb1/fb2/fb01) ---defaults is fb0
			-C (config file path)
			-I (number of iterations for fps test)

Kernel-Test compliance option:-
			-n (nominal test):-		./fbtest -n -v
			-a (adversarial test):-		./fbtest -a -v
			-r (repeatability test):-	./fbtest -r -v
			-s (stress test):-		./fbtest -s -v

Decimation option:-
	Enable decimation
		decimation
		./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t scale -h 44 -w 44 -S 0 -v decimation

CRC option:-
	CRC generation
			crc0 misr -e <CRC db file path>
	CRC validation
			crc1 misr -e <CRC db file path>

	MDP4 tests with parameters:

		For, -t scale
				-w (output Width)
				-h (output Height)
				-S (scale in step) [0,1]

		For, -t rotate
				-d (rotate degree) [0, 90, 180, 270]
				-f (flip mode) [LR, UD, LRUD/UDLR]

		For, -t crop
				-c (crop test mode) [25, 50, 75, TOP, BOTTOM, LEFT, RIGHT, RAND]
				-H (crop height for -c RAND)
				-W (crop width for -c RAND)
				-i (crop 'x'co-ordinate -c RAND)
				-j (crop 'y'co-ordinate -c RAND)

		For, -t move
				-x (move start'x' co-ordinate)
				-y (move start'y'co-ordinate)
				-X (move end 'x' co-ordinate)
				-Y (move end 'y' co-ordinate)

		For, -t dither or deinterlace
				on (dither or deinterlace on based on test selected)
				off (dither or deinterlace off. default value is off)

		For, -t video
				-F (fps of video to be played)

		For, -t blend or colorkey
				-1 (BG image file path for blend/colorkey)
				-2 (FG1 image file path for blend/colorkey)
				-3 (FG2 image file path for blend)
				-4 (FG3 image file path for blend)
				-P (FG1 alpha value for blend) [0 to 255]
				-Q (FG2 alpha value for blend) [0 to 255]
				-R (FG3 alpha value for blend) [0 to 255]
				-T (transparency value for colorkey) [0x00000000 to 0xFFFFFFFF]
				const (for constant alpha blending)
				pix (for pixel alpha blending)
				bg (for background colorkeying)
				fg (for forground colorkeying)

		For, -t multiop
				mcs (for move-crop-scale multi-operation)
				rmcs (for rotate-move-crop-scale multi operation)

		For, -t postproc
				--file (Configuration file for post processing test framework)
				For post processing test config file format,
				see "Post Processing Test Configuration File Format" section in this document
	MDP3 tests with parameters:

		For, -A Async Blit
				performs an async blit operation
		For, -Z Destination Dump
				Copies the destination to a file.
		Other parameters are same as MDP4.

Mandatory/Default parameter Details
-----------------------------------

	-m  mode of operation is user/auto
		user[1] -- user will take control & pass individual file
		auto[2] -- test will be automatic on set of files & need to pass the directory path of files

	-p  path of dircetory or name of file
		Input file path -- In case of user mode
		directory path  -- In case of auto mode

	-t  name or number of the test to be perform

		MDP4 tests names and numbers:
			allmdp4		[0]	---for testing all mdp4 features
			format		[1] 	---for format overlay test
			scale		[2] 	---for scaling test
			rotate		[3] 	---for rotation test
			crop		[4] 	---for crop test
			move		[5] 	---for move test
			deinterlace	[6] 	---for deinterlace test
			dither		[7]	---for dither test
			videoplay	[8]	---for video playback test
			fpsvsync	[9]	---for fps Vsync test
			blend		[10] 	---for blending test
			colorkey	[11]	---for colorkey test
			multiopt	[12]	---for multiopt test
			adversarial	[13]	---for negative testing
			hwcursor	[14]	---for hwcursor testing
			overlayscale	[15]	---for overlayscale testing
			overlayalpha	[16]	---for overlayalpha testing
			overlayStress	[17]	---for overlaystress testing
			allcolorFormatOverlay[18]---for all color format testing
			overlayARGB	[19]	---for overaly ARGB testing
			overlay		[20]	---for overlay testing
			csc		[21]	---for color space conversion testing
			postproc	[22]	---for display post processing testing

		MDP3 tests names and numbers:
			allmdp3				 [0] ---for testing all mdp3 features (legacy test case)
			format				 [1] ---for format overlay test
			scale				 [2] ---for scaling test
			rotate  	 		 [3] ---for rotation test
			crop    			 [4] ---for crop test
			move				 [5] ---for move test
			dither  			 [6]  ---for dither test
			videoplay			 [7] ---for video playback test
			fpsvsync    		         [8] ---for fps Vsync test
			blend      			 [9] ---for blending test
			colorkey			[10] ---for colorkey test
			multiop				[11] ---for multiopt test
			adversarial			[12] ---for negative testing
			overlay				[13] ---for overlay testing
			csc 				[14] ---for color space conversion testing
			postproc			[15] ---for display post processing testing
			/* legacy test cases */
			drawBG				[16] ---for drawing white background
			CalcFPS				[17] ---for calculating FPS
			PPP					[18] ---for ppp operations
			allColorformatPPP [19] ---for ppp operation on all color formats
			pppTestVideo		[20] ---for ppp operation on test video
			stress				[21] ---for stress test
			adversarial			[22] ---for adversarial test
			mddiPartial			[23] ---for mddi partial operation

	-b	framebuffer selection
		fb0 [0] ---Selects framebuffer 0  (primary interface)
		fb1 [1] ---Selects framebuffer 1  (external interface)
		fb2 [2] ---Selects framebuffer 2  (mixer-2/wi-fi display interface)
		fb01 [3] ---Selects framebuffer 0 & 1  (HDMI mirroring)
Note: MDP3 (drawBG, CalcFPS, PPP, allColorformatPPP, pppTestVideo, mddiPartial) tests runs with patterns as inputs, so, they require no input test images or parameters.
Note1: MDP3 (drawBG, CalcFPS, PPP, allColorformatPPP, pppTestVideo, mddiPartial) tests are deprecated on 8x10 SoC.
Note2: New MDP3 tests need command line arguments.

Examples:
---------
MDP3 test examples:

		drawBG test:
			./fbtest -t drawBG -v

		CalcFPS test:
			./fbtest -t CalcFPS -v

		PPP test:
			./fbtest -t PPP -v

		allColorformatPPP test:
			./fbtest -t allColorformatPPP -v

		pppTestVideo test:
			./fbtest -t pppTestVideo -v

		stress test:
			./fbtest -t stress -v

		adversarial test:
			./fbtest -t adversarial -v

		mddiPartial test:
			./fbtest -t mddiPartial -v

	For testing in user mode --

		Format test:
			./fbtest -m user -p  /sdcard/fasimo_352x288_rgb_888.rgb -t format -v -A
			./fbtest -m user -p  /sdcard/fasimo_352x288_rgb_888.rgb -t format -v
			./fbtest -m user -p  /sdcard/fasimo_352x288_rgb_888.rgb -t format -v -Z

		Scale Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t scale -h 800 -w 480 -S 0 -v
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t scale -h 800 -w 480 -S 0 -v -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 2 -h 800 -w 480 -S 1 -v
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 2 -h 800 -w 480 -S 1 -v -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 2 -h 800 -w 480 -S 1 -v -A -Z

		Rotate Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t rotate -d 90  -v
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t rotate -d 90  -v -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 3 -d 180  -v
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 3 -d 180  -v  -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 3 -d 180  -v  -Z

		Crop Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t crop -c 25 -v
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t crop -c 25 -v -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 4 -c RAND -i 20 -j 20 -H 100 -W 100 -v
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 4 -c RAND -i 20 -j 20 -H 100 -W 100 -v -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 4 -c RAND -i 20 -j 20 -H 100 -W 100 -v -Z

		Move Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t move -x 0 -y 0 -X 100 -Y 120 -v
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t move -x 0 -y 0 -X 100 -Y 120 -v -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 5 -x 90 -y 100 -X 10 -Y 120 -v
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 5 -x 90 -y 100 -X 10 -Y 120 -v -A
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 5 -x 90 -y 100 -X 10 -Y 120 -v -Z

		Dither Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither on -v
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither on -v -A
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither  -v
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither  -v -A
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither  -v -Z

		VideoPlay Test:
			./fbtest -m user -p /sdcard/akiyo_176x144_Y_CB_CR_H2V2.yuv -t videoPlay -F 30 -v
			./fbtest -m user -p /sdcard/akiyo_176x144_Y_CB_CR_H2V2.yuv -t videoPlay -F 30 -v -A
			./fbtest -m user -p /sdcard/akiyo_176x144_Y_CB_CR_H2V2.yuv -t videoPlay -F 30 -v -Z

		Blend Test:
			./fbtest -m user -t blend -v -1 yellowred_1024x600_rgba_8888.rgb -2 fasimo_352x288_argb_8888.rgb const
			./fbtest -m user -t blend -v -1 yellowred_1024x600_rgba_8888.rgb -2 fasimo_352x288_argb_8888.rgb const -A
			./fbtest -m user -t 10 -v -1 yellowred_1024x600_rgba_8888.rgb -2 fasimo_352x288_argb_8888.rgb  pix
			./fbtest -m user -t 10 -v -1 yellowred_1024x600_rgba_8888.rgb -2 fasimo_352x288_argb_8888.rgb  pix -A
			./fbtest -m user -t 10 -v -1 yellowred_1024x600_rgba_8888.rgb -2 fasimo_352x288_argb_8888.rgb  pix -Z

		ColorKey Test:
			./fbtest -m user -t colorkey -v -1 yellowred_1024x600_rgba_8888.rgb -2 greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF fg
			./fbtest -m user -t colorkey -v -1 yellowred_1024x600_rgba_8888.rgb -2 greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF fg -A
			./fbtest -m user -t 11 -v -1 yellowred_1024x600_rgba_8888.rgb -2 greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF bg
			./fbtest -m user -t 11 -v -1 yellowred_1024x600_rgba_8888.rgb -2 greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF bg -A
			./fbtest -m user -t 11 -v -1 yellowred_1024x600_rgba_8888.rgb -2 greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF bg -Z

		Multi-operation Test:
			./fbtest -m user -t multiop -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 mcs
			./fbtest -m user -t multiop -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 mcs -A
			./fbtest -m user -t 12 -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 -d 90 rmcs
			./fbtest -m user -t 12 -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 -d 90 rmcs -A
			./fbtest -m user -t 12 -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 -d 90 rmcs -Z

		Color Space Conversion Test:
			./fbtest -m user -p /sdcard/akiyo_176x144_Y_CB_CR_H2V2.yuv -t csc -F 30 -v

MDP4 test examples:

	For FPS-VSYNC test:
			./fbtest -t fpsvsync -v
			./fbtest -t 9 -v

	For Adversarial test:
			./fbtest -t adversarial -v
			./fbtest -t 13 -v

	For overlayscale test:
			./fbtest -t overlayscale -v
			./fbtest -t 15 -v

	For overlayalpha test:
			./fbtest -t overlayalpha -v
			./fbtest -t 16 -v

	For hwcursor test:
			./fbtest -t hwcursor -v
			./fbtest -t 14 -v

	For overlayStressTest:
			./fbtest -t overlayStress -v
			./fbtest -t 17 -v

	For allcolorFormatOverlayTest:
			./fbtest -t allcolorFormatOverlay -v
			./fbtest -t 18 -v

	For overlayARGBTest test:
			./fbtest -t overlayARGB -v
			./fbtest -t 19 -v

	For overlayTest test:
			./fbtest -t overlay -v
			./fbtest -t 20 -v

	For postproc test:
			./fbtest -t postproc --file <path to test configuration file on device>


	For testing in user mode --

		Format test:
			./fbtest -m user -p  /sdcard/fasimo_352x288_rgb_888.rgb -t format -v

		Scale Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t scale -h 800 -w 480 -S 0 -v
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 2 -h 800 -w 480 -S 1 -v


		Rotate Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t rotate -d 90 -f LR -v -D /sdcard/
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 3 -d 180 -f LRUD -v -D /sdcard/

		Crop Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t crop -c 25 -v
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 4 -c RAND -i 20 -j 20 -H 100 -W 100 -v

		Move Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t move -x 0 -y 0 -X 100 -Y 120 -v
			./fbtest -m 1 -p /sdcard/fasimo_352x288_rgb_888.rgb -t 5 -x 90 -y 100 -X 10 -Y 120 -v

		Dither Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither on -v
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither  -v

		Deinterlace Test:
			./fbtest -m user -p /sdcard/avtarinterlace_1280x720_Y_CB_CR_H2V2.yuv -t deinterlace on -v
			./fbtest -m user -p /sdcard/avtarinterlace_1280x720_Y_CB_CR_H2V2.yuv -t deinterlace  -v

		VideoPlay Test:
			./fbtest -m user -p /sdcard/akiyo_176x144_Y_CB_CR_H2V2.yuv -t videoPlay -F 30 -v

		Blend Test:
			./fbtest -m user -t blend -v -1 yellowred_1024x600_rgba_8888.rgb -2 fasimo_352x288_argb_8888.rgb -3 akiyo_176x144_Y_CB_CR_H2V2.yuv const
			./fbtest -m user -t 10 -v -1 yellowred_1024x600_rgba_8888.rgb -2 fasimo_352x288_argb_8888.rgb -3 akiyo_176x144_Y_CB_CR_H2V2.yuv pix

		ColorKey Test:
			./fbtest -m user -t colorkey -v -1 yellowred_1024x600_rgba_8888.rgb -2 greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF fg
			./fbtest -m user -t 11 -v -1 yellowred_1024x600_rgba_8888.rgb -2 greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF bg

		Multi-operation Test:
			./fbtest -m user -t multiop -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 mcs
			./fbtest -m user -t 12 -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 -d 90 -f LR rmcs

		Color Space Conversion Test:
			./fbtest -m user -p /sdcard/akiyo_176x144_Y_CB_CR_H2V2.yuv -t csc -F 30 -v

	For testing in auto mode --

		AllMDP4 test:
			./fbtest -m auto -t allmdp4 -p /sdcard/ -v

		Format test:
			./fbtest -m auto -p  /sdcrad/ -t format -v

		Scale test:
			./fbtest -m auto -p  /sdcard/ -t scale -v

		Roatate test:
			./fbtest -m auto -p  /sdcard/ -t 3 -v

		Crop test:
			./fbtest -m auto -p  /sdcard/ -t 4 -v

		Move test:
			./fbtest -m auto -p  /sdcard/ -t 5 -v

		Dither Test:
			./fbtest -m auto -p /sdcard/ -t dither on -v  (for on)
			./fbtest -m auto -p /sdcard/ -t dither  -v (for off)

		Deinterlace Test:
			./fbtest -m auto -p /sdcard/ -t deinterlace on -v (for on)
			./fbtest -m auto -p /sdcard/ -t deinterlace  -v (for off)

		videoPlay Test:
			./fbtest -m auto -p /sdcard/ -t videoPlay -F 30 -v

		Blend test:
			./fbtest -m auto -p /sdcard/ -t blend -v

		Multi-operation test:
			./fbtest -m auto -p /sdcard/ -t multiopt -v

		Color Space Conversion Test:
			./fbtest -m auto -p /sdcard/ -t csc -F 30 -v

Mixer-2 Test example:-
		Format test:
			./fbtest -m user -p  /sdcrad/fasimo_352x288_rgb_888.rgb -t format -v -b fb2 -D /sdcard/mixer2/

		Scale Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t scale -h 800 -w 480 -S 0 -v -b fb2 -D /sdcard/mixer2/

		Rotate Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t rotate -d 90 -f LR -v -b fb2 -D /sdcard/mixer2/

		Crop Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t crop -c 25 -v -b fb2 -D /sdcard/mixer2/

		Move Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t move -x 10 -y 10 -X 100 -Y 120 -v -b fb2 -D /sdcard/mixer2/

		Dither Test:
			./fbtest -m user -p /sdcard/fasimo_352x288_rgb_888.rgb -t dither on -v -b fb2 -D /sdcard/mixer2/

		Deinterlace Test:
			./fbtest -m user -p /sdcard/avtarinterlace_1280x720_Y_CB_CR_H2V2.yuv -t deinterlace on -v -b fb2 -D /sdcard/mixer2/

		Blend Test:
			./fbtest -m user -t blend -v -1 /sdcard/fasimo_352x288_rgb_888.rgb -2 /sdcard/fasimo_352x288_argb_8888.rgb -3 /sdcard/akiyo_176x144_Y_CB_CR_H2V2.yuv -P 60 -Q 60 -b fb2 -D /sdcard/mixer2/

		ColorKey Test:
			./fbtest -m user -t colorkey -v -1 /sdcrad/yellowred_1024x600_rgba_8888.rgb -2 /sdcrad/greenyellowwhite_1024x600_rgba_8888.rgb -T 0xFF00FFFF fg -b fb2 -D /sdcard/mixer2/

		Multi-operation Test:
			./fbtest -m user -t multiop -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 mcs -b fb2 -D /sdcard/mixer2/
			./fbtest -m user -t multiop -v -p /sdcard/fasimo_352x288_argb_8888.rgb -X 100 -Y 100 -c rand -i 100 -j 100 -W 150 -H 150 -w 500 -h 500 -d 90 -f LR rmcs -b fb2 -D /sdcard/mixer2/

		FPS-VSYNC test:
			./fbtest -t fpsvsync -v -b fb2 -D /sdcard/mixer2/



Post Processing Test Configuration File Format
----------------------------------------------

The framebuffer test post processing framework (display post processing test)
uses test configuration file to run display post processing tests. For example
to run post processing test with test.cfg test configuration file, following
command can be used from shell.

	./fbtest -t postproc -B test.cfg

Each line in test configuration file (For example in example abolve, test.cfg)
is interpreted as one step for post processing test. Test framework processes
one step at a time, so next step will be executed after current step is
executed. Each step of test can also be referred as "operation". Set of steps
(or operations) form one test case. Given below is format for specifying step
in test configuration file:

	<OP_TYPE> <<OP_PARAMETERS>>

<OP TYPE> supports 2 types of operations: SET or SLEEP.

Operation	OP_TYPE value
SET		    0
SLEEP		    1

SET Operation:
--------------

SET operation is used to configure the various post processing parameters.
When SET operation is specified, the OP_PARAMETERS are split into two parts
as mentioned below. One of these specifies the parameters to be configured,
and the other specifies the configuration values to be used.

<OP_TYPE> <OP_CODE> <<OP_CONFIGURATION>>

OP_TYPE supports 5 types of parameters currently:
	Operation Type					OP_TYPE Value
	---------------------------------------------------------------
	Set                                                 0
	Sleep                                               1
	Get                                                 2
	ThreadJoin                                          3
	Repeat                                              4

OP_CODE supports 11 types of parameters currently:

	Parameter Type					OP_CODE Value
	---------------------------------------------------------------
	Hue, Saturation, Intensiy & Contrast (HSIC)	    0
	Polynomial Color Correction (PCC)		    1
	Area Reduced Gamma Correction (AR-GC)		    2
	Inverse Gamma Correction (IGC)                      3
	Histogram Lookup Table (HistLUT)                    4
	Histogram Start                                     5
	Histogram Stop                                      6
	Histogram Read**                                    7
	Post processsing daemon test                        8
	QSEED Table test**                                  9
	Post processing calibration API test**              10
	Gamut Mapping					    11
	Picture Adujstment				    12
	Color Space Conversion				    13
	QSEED2 Sharp/smooth				    14
	Source Histogram start/stop			    15
	Assertive Display test                              16
	Picture Adjustment v2                               17

**: Except for those starred, all op_codes are supported only by the 'Set'
Operation type.

Each of these features has a unique format for OP_CONFIGURATION. It should be
noted that OP_CONFIGURATION is delimited by ';' instead of ' '.

For HSIC (CSC) :
NOTE: parameters in []'s are optional

<MDP_BLOCK>;<OPS>;<HUE>;<SATURATION>;<INTENSITY>;<CONTRAST>[;<INTERFACE>;<<RGB2YUV Conversion param file>>;<<YUV2RGB Conversion param file>>]

	The INTERFACE parameter is defined by the 'interface_type' enum in
	lib-postproc.h. The Conversion parameter files will contain 3 rows,
	each containing 3 decimal values. It directly correlates with a 3x3
	conversion matrix.

For PCC:

    There should be no spaces or newlines between the PCC arguments. The newlines are here for readability.

<MDP_BLOCK>;<OPS>;<R.c>;<R.r>;<R.g>;<R.b>;<R.rr>;<R.gg>;<R.bb>;<R.rg>;<R.gb>;<R.rb>;<R.rgb>;
                    <G.c>;<G.r>;<G.g>;<G.b>;<G.rr>;<G.gg>;<G.bb>;<G.rg>;<G.gb>;<G.rb>;<G.rgb>;
                    <B.c>;<B.r>;<B.g>;<B.b>;<B.rr>;<B.gg>;<B.bb>;<B.rg>;<B.gb>;<B.rb>;<B.rgb>

For Picture Adjustment
	hue          - Hue, valid from -180.0 to 180.0 degrees
	saturation   - Saturation, valid from -1.0 to 1.0 (percentage)
	value        - Value, valid from -255 to 255
	contrast     - Contrast, valid from -1.0 to 1.0 (percentage)

<MDP_BLOCK>;<OPS>;<HUE>;<SATURATION>;<VALUE>;<CONTRAST>

For Picture Adjustment v2

<MDP_BLOCK>;<OPS>;<<PAv2 params file>>


    OPS - Options for HSVC, memory color, and six zone color adjustment
	bit 0 - Enable/disable Picture Adjustment, 0 - disable, 1 - enable
	bit 1,2 - Read/write PA data
		  00 - Do not read/write
		  01 - Read HSIC data from regs
		  10 - Write HSIC data to regs
		  11 - Reserved
	- bits 4-11 specify enable/disable of the registers being read from or
		written to for that specific feature.
	bit 4 - Global hue adjust read/write enable, 0 -disable, 1 - enable
	bit 5 - Global saturation adjust read/write enable, 0-disable, 1-enable
	bit 6 - Global value adjust read/write enable, 0-disable, 1-enable
	bit 7 - Global contrast adjust read/write enable, 0-disable, 1-enable
	bit 8 - Six zone adjust read/write enable, 0-disable, 1-enable
	bit 9 - Memory color skin adjust read/write enable, 0-disable, 1-enable
	bit 10 - Memory color sky adjust read/write enable, 0-disable, 1-enable
	bit 11 - Mem col foliage adjust read/write enable, 0-disable, 1-enable

	- bits 12-21 specify enable/disable of the feature via mask bit even if
		the registers have been programmed
	bit 12 - Global hue adjust mask, 0-disable, 1-enable
	bit 13 - Global saturation adjust mask, 0-disable, 1-enable
	bit 14 - Global value adjust mask, 0-disable, 1-enable
	bit 15 - Global contrast adjust mask, 0-disable, 1-enable
	bit 16 - Six zone hue adjust mask, 0-disable, 1-enable
	bit 17 - Six zone saturation adjust mask, 0-disable, 1-enable
	bit 18 - Six zone value adjust mask, 0-disable, 1-enable
	bit 19 - Memory color skin adjust mask, 0-disable, 1-enable
	bit 20 - Memory color sky adjust mask, 0-disable, 1-enable
	bit 21 - Memory color foliage adjust mask, 0-disable, 1-enable

	bit 22 - Memory color adjustment protection, 0 - global adjustment
		happens independently of memory color, 1 - global adjustment
		allowed only if pixel hasn't been processed by memory color
	bit 23 - Saturation zero exponent enable, 0 - if global sat adjust < -32
		the sat adjust is clamped to 0, 1 - if global sat adjust < -64
		the sat adjust is clamped to 0.

	The PAv2 params file will contain parameters based on what picture
	adjustment features are desired. Each line will contain parameters
	separated by spaces (' '), with the first item on the line being the
	line "op" which are defined as follows:

	Line Description        Line Op      Parameters
	Global Adjustment       0            <hue> <saturation> <saturation thresh> <value> <contrast>
		Global adjustment elements specified below
			global hue          - Hue, valid from -180 to 180 degrees
			global saturation   - Saturation, valid from -1.0 to 1.0 (percentage)
			sat thresh          - Saturation adjustment threshold, valid 0 to 1.0
			global value        - Value, valid from -255 to 255
			global contrast     - Contrast, valid from -1.0 to 1.0 (percentage)

	Six Zone Hue LUT        1            <<384 hue values defining six zone hue adjustment curve>>
	Six Zone Saturation LUT 2            <<384 saturation values defining six zone saturation adjustment curve>>
	Six Zone Value LUT      3            <<384 value values defining six zone value adjustment curve>>
	Six Zone LUT Thresholds 4            <saturation min> <value min> <value max>

		Six Zone elements specified below
			hue - 384 Hue values specify the six zone hue adjust curve (-180 - 180)
			sat - 384 sat values specify the six zone sat adjust curve (-1.0 - 1.0)
			val - 384 val values specify the six zone val adjust curve (-1.0 - 1.0)
			sat min - threshold for six zone sat adjustment (0-1.0)
			val min - min val for six zone val adjustment (0-255)
			val max - max val for six zone val adjustment (0-255)

	Skin Memory Color Adj   5            <offset> <hue slope> <saturation slope> <val slope> <hue min> <hue max> <saturation min> <saturation max> <val min> <val max>
	Sky Memory Color Adj    6            <offset> <hue slope> <saturation slope> <val slope> <hue min> <hue max> <saturation min> <saturation max> <val min> <val max>
	Fol Memory Color Adj    7            <offset> <hue slope> <saturation slope> <val slope> <hue min> <hue max> <saturation min> <saturation max> <val min> <val max>

		Memory Color elements specified below
			offset - offset from minimum hue value to top corner of trapezoid (0 - 360)
			hue slope - floating point slope of trapezoid side for hue adjust, starting from hue min (-inf - inf)
			sat slope - floating point slope of trapezoid side for sat adjust, starting from hue min (-inf - inf)
			val slope - floating point slope of trapezoid side for val adjust, starting from hue min (-inf - inf)
			hue min - Min hue for memory color adjustments (0 - 360)
			hue max - Max hue for memory color adjustments (0 - 360)
			sat min - Min sat for memory color adjustments (0 - 1.0)
			sat max - Max sat for memory color adjustments (0 - 1.0)
			val min - Min val for memory color adjustments (0 - 255)
			val max - Max val for memory color adjustments (0 - 255)

	Example Line for global adjustment:
	# Global Adjustment (# for comments)
	0 -180 .5 25 -20 .6

	Example Line for Skin Memory Color adjustment:
	# Skin memory color adjustment
	5 5 0 -.1 0 110 150 0 255 0 255

For ARGC (PGC):

<MDP_BLOCK>;<OPS>;<<R Channel ARGC params file>>;<<G Channel ARGC params file>>;<<B Channel ARGC params file>>;

	AR-GC parameters file will contain 16 rows, each corresponding to one
	stage of AR-GC for given channel. Each row will have four elements in it.
	First element of each row indicates if linear segment is enabled or not,
	0 means disabled, 1 means enabled. Second element in row is start value
	on x axis, this is unsigned integer value in range 0 to 4095. Third
	element in row is slope of linear segment, this is unsigned double value.
	Fourth element in row is y axis offset for linear segment, its unsigned
	double value. If linear segment is disabled, rest of three elements in
	row can be set to 0.

For IGC and HistLUT:

<MDP_BLOCK>;<OPS>;<<params file>>

	IGC/HistLUT parameters file will contain 256 rows, each corresponding
	to a pixel input value. Each row will have 3 elements in it; c0's
	value being first, then c1, then c2. The range of these values for
	HistLUT parameters is 0-255; for IGC it is 0-4095.
	For hist lut, this test now supports both DSPP and SSPP locations
	(see: MDSS_PP_DSPP_CFG and MDSS_PP_SSPP_CFG).

For Histogram (Start/Stop/Read) & ThreadJoin

0 <Histogram Start> <MDP_BLOCK>;<FRAME_COUNT>;<BIT_MASK>;<NUM_BINS>
2 <Histogram Read> <MDP_BLOCK>;<NUM_BINS>;<LOOPS>
0 <Histogram Stop> <MDP_BLOCK>
3 <ThreadJoin>
	MDP_BLOCK specifies which block the histogram is to be tested.
	FRAME_COUNT is the number of frames to sample for each histogram
	interrupt.
	BIT_MASK is used to specify higher resolution of subsections of the
	histogram.
	NUM_BINS is used to tell the number of bins that have been allocated
	to recieve successfull histogram reads.
	LOOPS specifies the number of iterations to run for the read.
	ThreadJoin - can be empty

	For correct histogram collection, you must read the histogram after
	starting the histogram for that block and before stopping the
	histogram. Similarly, ThreadJoin should be used only after all started
	histogram collections have stopped. Each fbtest line for Histogram
	Read starts a new thread to try to read histograms concurrently. There
	is a maximum of 10 threads avaliable for histogram.

Each feature shares the <MDP_BLOCK> configuration. This corresponds to MDP
block of the feature to configure.

	MDP Block	Value
	-----------------------
	Layer Mixer 0	1
	Layer Mixer 1	2
	VG Pipe 1	3
	VG Pipe 2	4
	RGB Pipe 1	5
	RGB Pipe 2	6
	DMA P		7
	DMA S		8
	DMA E		9

For the features that have an <OPS> configuration, this corresponds to the
'ops' flag to be passed (often configuring the state of the feature). Details
regarding the specific values to provide for <OPS> can be found in header file
for the Post Processing Library 'lib-postproc.h'.

Note: Value for ops should be a decimal integer.

For Post processing daemon:

<daemon_command>;<<args>>;<<args>>

	The daemon commands are simple strings. Some commands need the extra args
	with them. For example, the pp:set:hsic command needs values of the hsic
	params. They are passed after command and delimited by semi-colon.

List of commands:
	cabl:on
	cabl:off
	cabl:status
	cabl:set
	pp:set:hsic
	pp:on
	pp:off
	ad:on
	ad:off
	ad:init
	ad:config
	ad:input

List of options for "cabl:set" command:
	Low;
	Medium;
	High;
	Auto;

Examples:
To run the postproc tests:
    $ ./fbtest -t postproc --file <config file>
Sample config to test the cabl:on and cabl:off commands:
    0 8 cabl:on
    1 5
    0 8 cabl:off
    1 5
Sample config to set saturation value to -1 and then reset:
    0 8 pp:set:hsic;0;-1;0;0
    1 5
    0 8 pp:set:hsic;0;0;0;0
    1 5


For QSEED Table test:

<MDP_BLOCK>;<TABLE_NUM>;<OPS>;<TABLE_LENGTH>;<<QSEED Parameter file>>

	OPS corresponds to the generic ops field for Post Processing features,
	(e.g. to write and enable set this to '5'=0b0101). QSEED Parameter
	file should have the same number of elements as TABLE_LENGTH.

For Calibration API test:
	<MDP_ADDRESS(decimal)>;<DATA>

	Note:not all address are accessable.Data should be able to be
	contained in a unsigned 32-bit variable.

For Source Histogram Start/stop test:
	<MDP_BLOCK>;<FRAME_COUNT>;<BIT_MASK>;<NUM_BINS>;<TIME_TO_RUN>

	MDP_BLOCK specifies which block the histogram is to be tested. This
	block contains the location (i.e. SSPP, DSPP), and pipe num arguments
	(optional, if not provided pipe will be allocated).
	FRAME_COUNT is the number of frames to sample for each histogram
	interrupt.
	BIT_MASK is used to specify higher resolution of subsections of the
	histogram.
	NUM_BINS is used to tell the number of bins that have been allocated
	to recieve successfull histogram reads.
	TIME_TO_RUN is used to specify the number of seconds the histogram
	should remain in the "start" state before being told to stop.
	This test is threaded and should have a thread join command to ensure
	that all the threads have stopped before exiting the test script.

For Assertive Display test:
	<ops>;<<init param file>>;<<cfg param file>>;<<input param file>>

	ops corresponds to the generic ops field for Post Processing features,
	(e.g. you should only be using OPS_ENABLE(1) or OPS_DISABLE(8) here).
	'init param file' should specify ALL the initialization parameters to
	use (ad_init.txt, in this directory, is an example format of the file
	and parameters required). 'cfg parameter file' should specify which mode
	the Assertive display should be in during the test as well as ALL the
	configuration parameters (some modes don't require some configuration
	params, but file parsing is easier to always parse everything). See the
	example	file ad_cfg.txt in this directory. Input parameter file is a
	list of integers to be passed in as input. Without verbose mode, these
	are passed in at 5Hz, but with verbose mode, a smaller list can be used
	and stepped through.

SLEEP Operation-
---------------
SLEEP operation is useful for inserting delays between two steps of post
processing test. For SLEEP operation, OP_PARAMETERS are just the amount of
time in seconds for the test to sleep. SLEEP operation is also useful for
allowing for time to visually inspect the results of a post processing command.

E.g. the following line would cause the test to sleep for 10 seconds:

1 10

REPEAT Operation-
-----------------
REPEAT operation is useful for executing the commands repeatedly.
The commands followed after REPEAT will be executed multiple times.

OP_TYPE for REPEAT is 4.

      4 <no of times> <no of command lines>

       E.g  4 1000 4
            0 8 cabl:on
            1 1
            0 8 cabl:off
            1 1
with the above instructions 4 testcases after repeat command will be
executed 1000 times.

Note: Nesting of Repeat commands are not supported currently.
We can have many Repeat commands in the script unless one repeat command doesn't fall
into others scope. The scope of Repeat command is defined by <no of command lines>

"#" Usage in the Script
-----------------------
Fbtest provides mechanism for comments in test configuration files to make
configuration files readable and easily understandable. '#' is comment
delimiter for fbtest configuration files. '#' can be used to comment out
complete line or part of single line. Currently there is no support for
multi-line comments.

If a line begins with '#' complete line is ignored If '#' exists in line,
part of line after '#' is ignored. Currently '#' escaping (using '\')
is not supported.

E.g  #0 0 3;3;60;0;0;0
     #1 1
      4 4 2 # Repeat 4 times 2 command lines
      0 0 3;3;60;0;0;0 # applying hue as 60

The additional configuration values for each feature can  generally be
interpreted from the additional arguments required for that specific feature as
documented in the post processing library header file 'lib-postproc.h'.

GET Operation
-----------------
GET operation is used for reading values in from the MDP post processing
features. Similar to the SET operation, GET's first parameter is the type of
the feature you want to read from. Types supported are:

	7 Histogram read
	9 QSEED table test
	10 Post processing calibration API test

For Histogram read:
	Please refer to the section above on histogram collection.
For QSEED Table test:

<MDP_BLOCK>;<TABLE_NUM>;<OPS>;<TABLE_LENGTH>;<<QSEED Parameter file>>

	OPS corresponds to the generic ops field for Post Processing features,
	(e.g. to read set this to '2'=0b0010). "QSEED Parameter" file will be
	created (or overwritten if it already exists) when reading values into
	it. Output file will be composed of rows of (up to) 4 register dump
	values. e.g. Reading the 1024 entries of QSEED Table 2 will result in
	a file of 4, 256 length columns filled in from left to right, top to
	bottom.

For Calibration API test:
	<MDP_ADDRESS(decimal)>;<junk>

	Note:not all address are accessable.

Default postproc Tests
----------------------
Fbtest facilitates default tests for post processing if config file is not
specified.

Ex: To Run postproc tests with default test configuration
	 ./fbtest -t postproc -v
	 ./fbtest -t 22 -v

Tests Covered in the default test configuration
Hue Test
Saturation Test
Intensity Test
Contrast Test
IGC Test
ARGC Test
PCC Test

Input Image Test File Location:
-------------------------------
\\sun\hydlnx\displayLinux\fbtest_enhance\test_files\

How to add new tests:-
----------------------
WIKI: http://qwiki.qualcomm.com/quic/UTF/UTF_Technology_Areas_Covering/UTF_Display/display_add_new_tests

Known Issues:-
-------------
WIKI: http://qwiki.qualcomm.com/quic/UTF/UTF_Known_Issues/UTF_Display_Issues
