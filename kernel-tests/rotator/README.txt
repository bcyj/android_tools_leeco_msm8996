=======================================================
msm_rotator test program
=======================================================

Description:
This is a test program to verify the msm_rotator kernel driver.  It
opens the rotator driver and rotates various image formats.  It can be run
interactively for more test options.

It requires msm_rotator driver (/dev/msm_rotator).
It requires msm_fb framebuffer (/dev/fb) driver.

Parameters:
It will except the following optional arguments:
	-n, --nominal		Run nominal tests
	-a, --adversarial	Run adversarial tests
	-s, --stress		Run stress tests
	-r, --repeat		Run repeat tests
	-i, --interactive       Pauses after drawing each image
				Interactively queries of
				more options
	-d, --dumpimgs          Dump raw images to files before &
				after each test
	-t [num], --tnum	Run single test
	-v, --verbose		Run with debug messages

Return:
It will return 0 if all test cases succeed otherwise it
returns a value < 0.

Targets:
	7x30, 8660
