Usage: spidevtest device [OPTIONS] [TEST_TYPE]...
Runs the tests specified by the TEST_TYPE parameters on the device.

These tests can run on all the targets that support SPI - 8x50 and 7x30, when
one has to make sure there are no conflict for its GPIOs.

If no TEST_TYPE is specified, then the nominal test is run.

OPTIONS can be:
  -v, --verbose         run with debug messages on

TEST_TYPE can be:
  -n, --nominal         run standard functionality tests
  -a, --adversarial     run tests that try to break the driver
  -s, --stress          run tests that try to maximize the capacity of the driver
  -r, --repeat          run 10 iterations of both the nominal and adversarial tests
  -h, --help            print this help message and exit

There are also options for manual testing:
User should indicate whether to run in loopback/aardvark mode, and also specify
a testnum. Separate tests exist for both modes.

	-t [num], --tnum	Run single test
	-l, --loopback		Run loopback tests
	-d, --aardvark		Run aardvark tests
	-z, --max_speed		Change maxspeed




SPI Ethernet Testing Documentation

Usage: spiethernettest.sh (no arguments)
       spiethernettest.sh [-n|--nominal] [--noping] [--noprobe]

	Only nominal test cases are supported at this time.

OPTIONS:
	Default: run nominal tests:
	--noping
		Disables the ping test
	--noprobe
		Disables the probe test

	(these options need to appear following -n)

TEST BEHAVIOR:
	* Verify that the KS8851 ethernet device and driver are present
	* Verify Qualnet connectivity

The test will check that the KS8851 SPI Ethernet device is present in the
system and that basic network connectivity can be achieved.

TARGETS:
	* 8960 presilicon targets (all tests)
	* Any ethernet targets (ping test only)

NOTES:
The ping test assumes connectivity to qualnet. The network will need to be
brought up before the test is run, but this generally is done by the startup
scripts. The ping test may be run on any target with networking support,
but the probe test specifically applies to the KS8851 driver and thus is
restricted to 8960 targets.
