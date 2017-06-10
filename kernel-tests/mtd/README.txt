===============================================================================
                               mtd_test.sh
===============================================================================

Purpose of the test:
--------------------
The purpose of this test is to test the Nand functionality both at the MTD and
Yaffs2 layer. This is the main test script that can be invoked with standard
options like --nominal, --stress, --release etc. depending on the kind of test
category that needs to be run, this test internally invokes mtd_yaffs2_test.sh
and mtd_driver_test.sh with the relevant arguments.

It is advisable to use mtd_test.sh only and not to invoke mtd_driver_test.sh or
mtd_yaffs2_test.sh directly. However the details of these internal test scripts
are also provided below for the sake of completeness.

Test Usage, Options:
--------------------
./mtd_test.sh -n, --nominal | -a, --adversarial | -s, --stress | -r, --release
        | -p, --repeatability | -h, --help

	-n = runs all the test cases that fall under "nominal" category
	-a = runs all the test cases that fall under "adversarial" category
	-s = runs all the test cases that fall under "stress" category
	-r = runs all the test cases that fall under "release" category
	-p = runs all the test cases that fall under "repeatability" category



Internal Test script Details:
-----------------------------

To test the functionality at the driver and the file system level mtd_test.sh
uses mtd_driver_test.sh & mtd_yaffs2_test.sh, internally. The details of each
of these tests are provided below.

mtd_yaffs2_test.sh: It tests the nand/onenand driver at YAFFS2 level. This
checks the consistency of the data read/write to nand flash through YAFFS2.
The usage information for this test is given below.

./mtd_yaffs2_test.sh

mtd_driver_test.sh: This test tests the nand driver functionality at the MTD
layer (independent of the file system). The usage information for this test
is given below.

./mtd_driver_test.sh  < test_to_run > < device_partition_number >

	<test_to_run> = oobtest OR pagetest OR subpagetest OR readtest
		OR stresstest OR speedtest
	<device_partition_number> = 0 OR 1
	eg: mtd_driver_test.sh subpagetest 1


Dependencies & Prerequisites:
-----------------------------

The internal test script mtd_driver_test.sh, relies on kernel test modules for
performing the test. These modules perform the test when they are insmod-ed.
These (.ko) files are available only if the Kernel is built with the following
support enabled.

Device Drivers

	--> Memory Technology Device (MTD) Support

					--> MTD tests support

Once the "MTD tests support" is enabled all the test modules (.ko files) are
available at the following location.

	'out/target/product/<target_name>/obj/KERNEL_OBJ/drivers/mtd/tests'

Before running the tests, these (.ko) files need to be copied from the above
location to device using ADB to PATH below '/data/kernel-tests/'


===============================================================================

