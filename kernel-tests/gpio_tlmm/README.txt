	GPIO Testing Documentation

gpio_tlmm test
===============
USAGE: ./gpio_tlmm.sh [Test Type] [Options]

[Test Type]

  -n, --nominal		     Enable / disable GPIOs that are expected to be useable
  -a, --adversarial	     Try and Enable GPIOs that should prohibited
  -s, --stress		     Run nominal and adversarial tests 100 times each
  -r, --repeat		     Run the nominal test case 10 times

[Options]

  -f, --config		     Full path to configuration file
  -t, --target <name>	     Target name (eg. 7X30, 8X50, 7X27, 8X60)
  -v, --verbose <verbosity>  Run with test information displayed
  -h, --help		     Print help message and exit

if no 'test type' is specified, then the nominal case is executed

DESCRIPTION:

For gpio_tlmm.sh, it exercises the debugfs testing interface to the gpio driver. Upon
writing to the debugfs files, the driver issues a proc_comm request to the
modem to perform the requested operation -- that is, to enable or disable
the GPIO number in question.

The modem has a concept of a 'non-remotable' GPIO.  This is a GPIO which is
not allowed access via the proc_comm API.  Consequently, enable requests to
these GPIOs should fail by design.  The test verifies this.  It also verifies
that enabling a GPIO outside of the respective target's GPIO range
(eg. last_gpio+1) will fail.

The test reads the configuration data for each target from a configuration
file.  By default, this file is ./gpio_tlmm.conf.  The syntax is as follows:

For each target:

target_name
{
	VALID=number_set
	INVALID=number_set
	NON_RMT=number_set
}

where 'target_name' is a 4-char string: eg. 7X30, 8X50, 7X27
where 'number_set' is a pattern: 'a,b,c-f,g-j,z'

TARGETS:

gpio_tlmm.sh test is intended to be run on targets:
  -7x30 SURF / FFA / FLUID
  -8650 SURF / FFA
  -7627 SURF / FFA

NOTES:

Currently the modem on the 7x30 targets does not check for "non-remotable"
GPIOs.  This particular test case is disabled in the config file until the
support is added to the modem.  It must be updated as soon as it is supported.

If test runs on adb shell, then busybox installation is required since test uses
utilities like grep, sed, etc.


gpio_lib test
=============
USAGE: ./gpio_lib.sh  [Test Type] [Options]

[Test Type]

  -n, --nominal		     Enable / disable GPIOs that are expected to be useable
  -a, --adversarial	     Try and Enable GPIOs that should prohibited

[Options]

  -f, --config		     Full path to configuration file
  -t, --target <name>	     Target name (eg. 7X30, 8650, 7X27, 8660)
  -v, --verbose <verbosity>  Run with test information displayed
  -h, --help		     Print help message and exit

if no 'test type' is specified, then the nominal case is executed

DESCRIPTION:

For gpio_lib.sh, it looks for sysfs for GPIO and checks if the gpio chip had probed
and booted and claimed to have the expected number of lines. It also verifies that
a gpio chip that shouldn't be probed really isn't. e.g. (last_gpiobase + 1)

The test reads the configuration data for each target and device if necessary from
a configuration file. By default, this file is ./gpio_lib.conf. The syntax is as follows:

For each target:

target_name
{
	LABEL=name_set
	BASE=number_set
	NGPIO=number_set
	INVALID=number_set
}

or

if for the same target, different device types have different gpio settings:

target_name
{
	Device1_LABEL=name_set
	Device1_BASE=number_set
	Device1_NGPIO=number_set
	Device1_INVALID=number_set

	Device2_LABEL=name_set
	Device2_BASE=number_set
	Device2_NGPIO=number_set
	Device2_INVALID=number_set

	...
}

where 'target_name' is a 4-char string: eg. 7X30, 8650, 7X27, 8660
where 'name_set' is a set of names of gpio labels: eg. msmgpio, sx1509q
where 'number_set' is a set of integer numbers
where 'Device#' is the name of the device: FFA, SURF, FLUID

TARGETS:

gpio_lib.sh test is intended to be run on targets:
  -7x30 SURF / FFA / FLUID
  -8650 SURF / FFA
  -7627 SURF / FFA
  -8660 SURF / FFA / FLUID
  -8960 RUMI
  -8974 VIRTIO
  -8084 VIRTIO
  -9625 SURF / FFA
  -8610 VIRTIO
  -8x10 VIRTIO
  -8226 VIRTIO
  -8x26 VIRTIO
  -8026 VIRTIO
  -8926 VIRTIO
  -8962 VIRTIO / RUMI
  -8092 VIRTIO
  -8916 VIRTIO / RUMI
  -8939 VIRTIO / RUMI
  -8909 VIRTIO / RUMI

NOTES:

1) If test runs on adb shell, then busybox installation is required since test uses
utilities like grep, sed, etc.
2) Target type auto-detection relies on test_env_setup.sh so TEST_ENV_SETUP needs to be
set to point to that script. This should be done by the user of the test script before
invoking this test if this setup haven't done this before.
e.g. export TEST_ENV_SETUP=path-to-test_env_setup.sh
3) For 8960, it looks for soc cpu id information to detect the target type. The assumed
path to the id information is /sys/devices/system/soc/soc0/id However, if that information
is missing, we can pass -t 8960 as argument when invoke the test.
e.g. ./gpio_lib.sh -t 8960
