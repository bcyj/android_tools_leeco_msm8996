	Timer / Clocksource Testing Documentation

Usage: timertest.sh (no arguments)
Runs the basic timer/clocksource test

OPTIONS:
  (none)

TEST BEHAVIOR:
	* Verify that there are two clocksources present
	* Attempt to switch to every clocksource
	* Verify that processes can be spawned without lockups regardless of
	  the clocksource used.
The test will go through both clocksources and switch to them one at a time,
spawning a process (ls) after each switch. If clocksource switching is not
implemented correctly, this is known to cause a lockup.

TARGETS:
	* 8960 Virtio / 8960 RUMI
	* 8660 silicon targets

NOTES:
If the script fails to run to completion (and the target hangs), then
this indicates a failure. Failures can occur due to bugs in the timer
code that prevent switching clocksources and using them for system timing
events. The test is currently known to fail on 8660 / 8960, though the
behavior being tested needs to be supported.


