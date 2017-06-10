	SMP CPU Testing Documentation

Usage: cputest.sh -c <number of CPUs>
Runs the basic CPU test

OPTIONS:
  (none)

TEST BEHAVIOR:
	* Verify that the CPU type was detected as ARMv7 in /proc/cpuinfo
	* Verify that all CPU cores have come up in /proc/cpuinfo

DESCRIPTION:

The test assumes that none of the CPUs had been hot-removed and that SMP is
enabled. A failure of the SMP check can occur if CPUs are not up for one of
a number of reasons, such as misconfiguration, SCM problem, lockup, lack of
interrupts on the secondary CPU, etc. A failure of the ARMv7 check can be due to
improper KraitMP detection in the generic ARM code, missing MMFR0 detection
patches, or unexpected values in the generic CPU identification registers.

TARGETS:
	8960 Virtio / 8960 RUMI / 8064 / 8974 / 8610 / 8x10 / 8226 / 8026 / 8x26 / 8084 / 8916 / 8939 /8936 / 8909, SMP enabled
	8660 silicon targets, SMP enabled
        8092 Virtio ,SMP enabled
	The test WILL fail on any non-SMP / non-ARMv7 target

NOTES:
This test can potentailly fail on any target that has CPU hotplugging and/or
any form of MP-decision which may disable one of the processors during runtime.
The test may still be used on the target, as long as all CPU hotpluging
mechanisms are deactivated and any CPUs that may have been brought down have been
restored to the online state.


cpu hotplug test
=================
Usage:
./cpuhotplug_test.sh [-n | --nominal] [-r | --repeatability ] [(-v | --verbosity) <verbosity>]

Description:
This test tests cpu hotplug functionality for SMP system.

Test logically turns on/off cpuX by changing the values /sys/devices/system/cpu/cpuX/online
and check if /proc/cpuinfo and /proc/interrupts respond correctly for corresponding changes.

Targets:
8660, 8960

Notes:
(1) On Android adb shell, busybox installation is required for test to run.
(2) Since cpu0 is not allowed to be hot plugged, the test intends to run on multiple cpus system
