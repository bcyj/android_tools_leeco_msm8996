#!/bin/sh
echo "Serial test starting"

cd /sys/devices/platform


echo "Looking for ttyMSM0"
C=`find |grep ttyMSM0 |wc -l`

if [ "$C" == "0" ]
then
	echo "Could not find ttyMSM0"
	echo "Test failed."
	exit 1
fi

echo "Serial test passed."

