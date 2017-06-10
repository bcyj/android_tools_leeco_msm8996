Test: Sanity test for socinfo

Usage: socinfotest.sh (no arguments)
Runs the basic socinfo sanity test

Options:
  (none)

Test Behavior:
	* Verify that socinfo is present in sysfs
	  - Verify that the build_id node is present
	  - Verify that the id node is present
	  - Verify that the version node is present
	* Verify that the CPU ID is not UNKNOWN (id is nonzero)

Targets:
	* All socinfo targets

Notes:
The test does not check that the socinfo nodes contain *correct* information,
beyond checking that the CPU is not unknow. This is due to the difficulty of
determining which target the test is running on besides using socinfo itself,
which would not be very effective. It would be up to the test envionment to
check the target type build version against the build currently loaded, as
it would know this information out of band.
