#-----------------------------------------------------------------------------
# Copyright (c) 2009-2010 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

echo "SPI Ethernet test starting"

tests="$*"

pingtest=0
probetest=0

if [ "x$tests" == "x" ]
then
	echo "Running nominal tests."
	tests='-n'
fi

for i in $tests
do
	case $i in
		-n|--nominal)
			pingtest=1
			probetest=1
			;;

		--noping)
			pingtest=0
			;;
		--noprobe)
			probetest=0
			;;
	esac
done

if [ "$probetest" != "0" ]
then
	echo "Running probe test"
	count=`cat /sys/bus/spi/drivers/ks8851/spi*/uevent |grep ks8851 |wc -l`
	if [ "$count" != "0" ]
	then
		echo "* Probe test passed."
	else
		echo "* Probe test failed!"
		exit $err
	fi
fi

if [ "$pingtest" != "0" ]
then
	echo "Running ping test"
	ping -c 1 qualnet > /dev/null
	err=$?

	if [ "$err" != "0" ]
	then
		echo "* Ping test failed! Error code $err"
		exit $err
	else
		echo "* Ping test passed."
	fi
fi

echo "SPI Ethernet test passed."
