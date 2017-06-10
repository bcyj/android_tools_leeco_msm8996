#-----------------------------------------------------------------------------
# Copyright (c) 2010-11 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

# Unit test for the UARTDM driver.

. $TEST_ENV_SETUP
# use this file as test data
test_file="$0"
#default HSUART node : /dev/ttyHS0
devname="/dev/ttyHS0"

while [ $# -gt 0 ]; do
	case $1 in
	-n | --nominal)
		shift 1
		;;
	-d)
		devname=$2
		shift 2
		;;
	-h | --help | *)
		echo "Usage: $0 [-n | --nominal] [-d <device node>]"
		echo "-d <device node> is optional. default: /dev/ttyHS0"
		exit 1
		;;
	esac
done

echo "HSUART: Nominal Testcases"
echo "========================="

# Test first argument if specified or ttyHS0 by default
if [ $devname = "/dev/ttyHS0" ]
then
    DEV_ID=0
elif [ $devname = "/dev/ttyHS1" ]
then
    DEV_ID=1
else
    echo "Error: Invalid Device node value"
    exit 1
fi

CLOCK_NODE="/sys/devices/platform/msm_serial_hs.$DEV_ID/clock"
LOOPBACK_NODE="${TEST_DEBUG_FS}/msm_serial_hs/loopback.$DEV_ID"

echo $CLOCK_NODE $LOOPBACK_NODE

echo "Testing " + $devname

if ! [ -f $CLOCK_NODE ]
then
  echo "Error: Unable to find clk switch node"
  exit 1
fi

if ! [ -f $LOOPBACK_NODE ]
then
  echo "Error: Unable to find loopback switch node"
  exit 1
fi

echo "Running Test Case:1 "
# create data file of size 1K
# 100k takes too long to finish, let's use 100 for normal test
#dd if=/dev/zero of=$TEST_TMP/tmpfile bs=1k count=100 2> /dev/null
dd if=/dev/zero of=$TEST_TMP/tmpfile bs=1 count=100 2> /dev/null

# send data file one by one
for i in $(seq 1 10)
do
cat $TEST_TMP/tmpfile > $devname
	if [ $? -ne 0 ]
	then
	    echo "Error in writing to device"
	    echo "Test Case 1: Failed"
	    exit 1
	fi
done
echo "Test Case 1: Passed"
echo "Running Test Case:2 "
# send data file in parallel
for i in $(seq 1 10)
do
cat $TEST_TMP/tmpfile > $devname &
	if [ $? -ne 0 ]
        then
	    echo "Error in writing to device"
	    echo "Test Case 2: Failed"
	    exit 1
	fi
done
sleep 1
killall cat 2>/dev/null
echo "Test Case 2: Passed"

echo "Running Test Case: 3 using UART Loopback Mode"
# echo off and post processing off
if [ "$TEST_TARGET" = "ANDROID" ]
then
	busybox stty -F $devname 200 -opost -echo
else
	stty -F $devname 200 -opost -echo
fi

# enable internal loopback mode in HW
echo 1 > $LOOPBACK_NODE

# send data file and recieve data into file
# match contents of sent and recieved files
cat $devname > $TEST_TMP/rxdata &
if [ $? -ne 0 ]
then
	echo "Test Case 3: Failed !"
	echo 0 > $LOOPBACK_NODE
	exit 1
fi
pid=$!
sleep 2
cat $test_file > $devname
sleep 2
# Send EOF character to terminate reading processes
echo -e '\004' > $devname
echo "process to kill" $pid
kill -9 $pid 2>/dev/null
diff $TEST_TMP/rxdata $test_file
if [ $? -ne  0 ]
then
	failed=1
else
	failed=0
fi

if [ "$TEST_TARGET" = "ANDROID" ]
then
	busybox rm -rf $TEST_TMP/tmpfile
else
	rm -rf $TEST_TMP/tmpfile
fi

# disable internal loopback mode in HW
echo 0 > $LOOPBACK_NODE
if [ $failed -gt 0 ]; then
	echo "Test Case 3: Failed!"
	exit 1
else
	echo "Test Case 3: Passed!"
	echo "Test Cases Completed."
fi
