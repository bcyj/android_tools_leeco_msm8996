#!/bin/sh
echo "Timer test starting"

cd /sys/devices/system/clocksource/clocksource0/
CS=`cat available_clocksource`

NUM=`cat available_clocksource |tr ' ' '\n' |wc -l`

# 2 clocksources plus one newline = 3
if [ "$NUM" != "3" ]
then
	echo "Unexpected number of clocksources (there should be 2)."
	echo "Test failed."
	exit 1
fi


echo "Available clocksources:"
echo $CS
echo

CURRENT_CS=`cat current_clocksource`
echo "Current clocksource is " $CURRENT_CS

echo "Trying all clocksources. If this hangs, the test fails."

for i in $CS
do
	echo "Switching to clocksource " $i
	echo $i > current_clocksource
	echo "Waiting 2 seconds"
	sleep 2
	ls & > /dev/null
done

# Do this twice depending on which CS was initially active.
for i in $CS
do
	echo "Switching to clocksource " $i
	echo $i > current_clocksource
	echo "Waiting 2 seconds"
	sleep 2
	ls & > /dev/null
done

echo "Restoring current clocksource to " $CURRENT_CS
echo $CURRENT_CS > current_clocksource

echo "Timer test passed."

