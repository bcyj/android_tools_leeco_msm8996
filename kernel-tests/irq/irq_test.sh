#-----------------------------------------------------------------------------
# Copyright (c) 2008-2011 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

TEST_TMP="$ANDROID_DATA/tmp"

. $TEST_ENV_SETUP

clean_up(){
    rm $TEST_TMP/1 > /dev/null 2>&1
    rm $TEST_TMP/2 > /dev/null 2>&1
}

clean_up
cat /proc/interrupts | grep timer > $TEST_TMP/1
sleep 5
cat /proc/interrupts | grep timer > $TEST_TMP/2
diff -q $TEST_TMP/1 $TEST_TMP/2 > /dev/null
if [ $? -eq 0 ]; then
	echo "Test Failed"
	clean_up
	exit 1
fi
clean_up
echo "Test Passed"
