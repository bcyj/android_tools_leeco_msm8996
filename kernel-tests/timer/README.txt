Timer Testing Documentation

Usage: timertest.sh
Runs the basic timer test

OPTIONS:
	* Timer test duration as an argument

TEST BEHAVIOR:
	* Verify (in a user-assisted manner) the timer tick rate is correct, defaults to 60 sec duration.

TARGETS:
	* All targets

USAGE:
	* ./run.sh [time in seconds]
	* This would make the system sleep for specified number of seconds, we have to manually verify
	* this duration with a stopwatch.

NOTES:
This test cannot be run in an automated way, as it requires and external user
to determine if we slept for the correct amount of time.

Advised usage :
On many pre-silicon targets the actual sleep function takes more time than the anticipated sleep
duration, this does not mean that the timer is broken so we recommend a difference test in such
situations, where you run the test with the two different durations and verify the difference.
For example, you run the test for 10 seconds and 20 seconds, suppose the test actually runs for
10 + x seconds in the case where you specify the duration as 10 , so an expected behaviour in a
good scenario would be that the test takes 20 + x seconds in the case a duration of 20 is used.
