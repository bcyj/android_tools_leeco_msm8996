	Cacheflush Test Documentation

Usage: cacheflush.sh
	No arguments or -n will run the Nominal test
	Passing -s will run the Stress test
	(The Stress test is a longer version of the Nominal test)


TEST BEHAVIOR:
	The test exercises the cache coherency logic for self-modifying code by
	continuously copying two alternate sets of instructions into a single
	memory buffer and executing the buffer. The associated TLB operations
	are also exercised, as the buffer is located at the same virtual
	address for every instance of the test program.

NOTES:
	Running the 'nominal' test will run 2 concurrent processes, each one
	running 1500 iterations of the self-modifying code test.

	Running the 'stress' test will run 5 such processes, each one running
	5000 iterations.

TARGETS:
	* Any ARMv7 target (7x27A, 7x30, 8x50, 8x60, 8960)
