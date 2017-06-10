#!/bin/sh
echo "Socinfo test starting"

if [ -d /sys/devices/soc0 ]; then
	cd /sys/devices/soc0
else
	cd /sys/devices/system/soc/soc0
fi

echo "Looking for build_id"
C=`find |grep build_id |wc -l`

if [ "$C" == "0" ]
then
	echo "Could not find build_id"
	echo "Test failed."
	exit 1
fi

echo "Looking for soc_id/id"
if [ -f soc_id ]; then
	C=`find |grep soc_id |wc -l`
else
	C=`find |grep id |wc -l`
fi

if [ "$C" == "0" ]
then
	echo "Could not find soc_id/id"
	echo "Test failed."
	exit 1
fi

echo "Looking for version/revision"
if [ -f revision ]; then
	C=`find |grep revision |wc -l`
else
	C=`find |grep version |wc -l`
fi

if [ "$C" == "0" ]
then
	echo "Could not find version/revision"
	echo "Test failed."
	exit 1
fi


echo "Checking that SOC_ID/ID is not 'unknown'"
if [ -f soc_id ]; then
	C=`cat soc_id`
else
	C=`cat id`
fi

if [ "$C" == "0" ]
then
	echo "CPU type is 'unknown'!"
	echo "Test failed."
	exit 1
fi

echo "Socinfo test passed."
