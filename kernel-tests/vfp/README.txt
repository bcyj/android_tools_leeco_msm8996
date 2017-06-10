Test: VFP unit

Usage: vfp.sh (no arguments)
Runs the basic VFP test

Options:
  (none)

Test Behavior:
	* Verify that floating point emulation is disabled
	* Verify that VFP support is enabled in the kernel
	* Run a math test to check basic floating-point operations
Targets:
	Any Scorpion / Krait target (8x50 / 7x30 / 8x60 / 8960)

Notes:
	The math test is written in C and is done using whatever instructions
	are inserted by the compiler. If floating point emulation is disabled,
	(the test checks for this), then hardware floating point support must
	be used.
	The test does not do exhaustive validation of the VFP unit, as this
	would have already been done by the processor team. The main purpose
	of this test is to check that floating-point operations can be carried
	out even in the absence of floating-point emulation support.
