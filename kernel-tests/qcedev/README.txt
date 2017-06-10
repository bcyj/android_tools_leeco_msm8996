			QCEDEV Test Application
			========================
		      Subsystem: kernel-tests/qcedev
		      ================================
Description:
===========
	The test application tests different crypto ci[her and hash algorithms.
The test are categorized as nominal, adversial, repetability and stress.
The algorithms that are tested are:
 CIPHER: AES-128 (CBC/ECB/CTR?XTS) and AES-256 (CBC/ECB/CTR/XTS)
 HASH:  SHA1, SHA256 and AES-MAC, CIPHER(AES and DES), CMAC, SHA256.

Dependencies:
=============
The test application requires the following modules to be either built-in kernel
or KLM (kernel loadable modules).
 qce.ko   -> Needed on msm8660, msm7x30 targets.
 qce40.ko -> Needed on msm8960 target
 qcedev.ko (driver module)

For loadable modules, the module needs to reside in data/kernel-tests directory.
These modules are located in
<BUILD_ROOT>/out/target/product/<target/obj/KERNEL_OBJ/drivers/crypto/msm
(<target> -> msm8660_surf, msm7x30_surf, msm8960 etc.)

Before running the test application, the drivers need to be "adb push"'ed to
data/kernel-tests.  Once it is available it needs to be loaded using "insmod".
If the ce driver modules are built-in,  you can proced to running the test
without having to "adb push " them.

(The above information is available from the help menu "./qcedev_test -h")

Pushing the drivers(qce.ko/qce40.ko and qcedev.ko) onto the device:
===================================================================
	adb push out/target/product/<target>/obj/KERNEL_OBJ/drivers/crypto/msm/
qce.ko /data/kernel-tests/
	adb push out/target/product/<target>/obj/KERNEL_OBJ/drivers/crypto/msm/
qcedev.ko /data/kernel-tests/

Connecting to the device:
=========================
From command shell,
	do 'adb shell'.
	e.g:    c:\>adb shell
		#

Loading the modules in the kernel:
====================================
- Load QCE module: #insmod qce.ko     (or qce40.ko)
- Load QCEDEV module: #insmod qcedev.ko

Running the Test Application:
=============================
- Change directory to data/kernel-tests
	#cd data/kernel-tests
- Change permission for qcedev_test:
	#chmod 777 qcedev_test'
- Run qcedev_test - See Usage section below for the parameters:
	#./qcedev_test -<OPTION> -<TEST_TYPE0> -<TEST_TYPE1> ..-<TEST_TYPEn]

Usage:
======
qcedev_test -[OPTION] -[TEST_TYPE0]..-[TEST_TYPEn]
	Runs the user space tests specified by the TEST_TYPE parameter(s).

	OPTION can be:
	-v, --verbose         run with debug messages on TEST_TYPE can be:

		  -n, --nominal         run standard functionality tests
		  -a, --adversarial     run tests that try to break the
		                          driver
		  -r, --repeatability   run 200 iterations of both the
		                          nominal and adversarial tests
		  -s, --stress          run tests that try to maximize the
		                          capacity of the driver
		  -p, --performance     run cipher and sha256 performance
					  tests.
		  -h, --help            print this help message and exit


