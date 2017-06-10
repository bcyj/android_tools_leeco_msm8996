#-----------------------------------------------------------------------------
# Copyright (c) 2009-2010, 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

#  Unit test for the I2C-MSM driver.
#
# Invoke by i2c-msm-test.sh
# This will run nominal testcases.

. $TEST_ENV_SETUP

test_opts=""
i2c_test="./i2c-msm-test"

# Build location
if [ -f /sys/devices/soc0/build_id ]; then
	export build_loc="/sys/devices/soc0/build_id"
else
	export build_loc="/sys/devices/system/soc/soc0/build_id"
fi
export i2c_target=`cat $build_loc | \
	awk -F= '{ print substr($0, 0, 4) }'`

# Function called to test a device
#probes to make sure that EEPROM is present on the target
#if present, runs nominal tests on the target using EEPROM
# $1 - the name of the device to test
# $2 - test options (if any)
#
# Returns 0 on success or 1 for failure
test_device()
{
	echo "$i2c_test --device $1 $2"
	$i2c_test --device $1 $2

	case "$?" in
		0) return 0;;
		*) return 1;;
	esac
}

#Based on build location, use appropriate I2C device
if [ ""$i2c_target == "8250" ] || [ ""$i2c_target == "8650" ]
then
	export device="/dev/i2c-1"
elif [ ""$i2c_target == "8260" ] || [ ""$i2c_target == "8660" ]
then
	export device="/dev/i2c-3"
elif [ ""$i2c_target == "8960" ]
then
	export device="/dev/i2c-10"
elif [ ""$i2c_target == "9x25" ]
then
	export device="/dev/i2c-3"
elif [ ""$i2c_target == "9x35" ]
then
	export device="/dev/i2c-1"
else
	echo "Assuming that EEPROM is located on I2C bus 0"
	export device="/dev/i2c-0"
fi

if test_device $device $test_opts
then
	# Test passed
	echo "Test passed"
	exit 0
else
	# Test failed
	echo "Test failed"
	exit 1
fi
