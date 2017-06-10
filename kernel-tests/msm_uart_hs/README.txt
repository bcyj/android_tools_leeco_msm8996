
MSM High Uart Unit Test
=======================

Using uarttest.sh script :
==========================
This script test the High Speed UART driver functionality as well as does provide
mechnism to do stress and stability testing of the same. It uses internal loopback
mode for HSUART Hardware to run all mentioned test cases.

1. Writing data file multiple time one by one ( Nominal Testcase )
2. Writing data file multiple time in parallel( Nominal Testcase )
3. Enabling internal loopback mode and doing data integrity check by reading data and comparing
same with written data.( Nominal Testcase )

Targets:
=======
All platform on which HSUART support is enabled. 7x27, 7x30, 8x50, and 8x60.
For all target HSUART device node is /dev/ttyHS0.

Prerequisite:
============
1. Make sure that your target is in Surf mode.
2. Make sure that busybox environment has been set and debugfs is mounted using test_env_setup script.

How-To Run:
==========
1. Run uarttest.sh from adb shell as
$ uarttest.sh [ -n | --nominal ] [ -d <device node> ]
device node (HSUART_NODE) value can be /dev/ttyHS0 or /dev/ttyHS1 ( in case if any future platform has this HSUART node).
Nominal test cases take /dev/ttyHS0 as default value if it is not provided on commandline.
By default uarttest.sh uses nominal test cases.

Result:
======
It runs all above test cases and provides the result of the same. It displays any error
if it has encountered while running the test case. For each test case, it displays result as 
"Test Passed" or "Test Failed".
====================================================================================================
