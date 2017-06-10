#-----------------------------------------------------------------------------
# Copyright (c) 2008-2011 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

# Script to test the voltage regulator driver.

# This may be either the vreg driver or the regulator driver depending upon
# the device.

TEST_DEBUG_FS="$ANDROID_DATA/debug"

. $TEST_ENV_SETUP

# Tests for MSM 8660 which uses the regulator driver.

test_regulator_enable() {
	local vreg=$1
	local v_low=$2
	local v_high=$3
	local load_uA=$4

	echo "$v_low $v_high" > $vreg/voltage
	if [ $load_uA -gt 0 ]; then
		echo $load_uA > $vreg/optimum_mode
	fi
	echo 1 > $vreg/enable
	local enabled=`cat $vreg/enable`
	local voltage=`cat $vreg/voltage`
	if [ $load_uA -gt 0 ]; then
		local mode=`cat $vreg/mode`
	fi

	if [ "$verbosity" -gt 0 ]; then
		if [ $load_uA -gt 0 ]; then
			echo "$vreg:  uV_req_range=[$v_low, $v_high]; after enabling: enable=$enabled, voltage=$voltage, mode=$mode"
		else
			echo "$vreg:  uV_req_range=[$v_low, $v_high]; after enabling: enable=$enabled, voltage=$voltage"
		fi
	fi

	if [ $enabled -ne 1 ]; then
		echo "FAILED: enable $vreg"
		failed=$(($failed + 1))
	fi
	# This check could fail if a voltage is specified that falls between
	# two levels which can actually be set in hardware and the regulator
	# driver rounds down to the lower one.
	if [ $voltage -lt $v_low ]; then
		echo "FAILED: set voltage $vreg"
		failed=$(($failed + 1))
	fi
}

test_regulator_disable() {
	local vreg=$1
	local require_disable=$2
	local v_high=$3
	local load_uA=$4

	echo 0 > $vreg/enable
	echo "0 $v_high" > $vreg/voltage
	if [ $load_uA -gt 0 ]; then
		echo 0 > $vreg/optimum_mode
	fi

	local enabled=`cat $vreg/enable`
	local voltage=`cat $vreg/voltage`
	if [ $load_uA -gt 0 ]; then
		local mode=`cat $vreg/mode`
	fi
	if [ "$verbosity" -gt 0 ]; then
		if [ $load_uA -gt 0 ]; then
			echo "$vreg:  after disabling: enable=$enabled, voltage=$voltage, mode=$mode"
		else
			echo "$vreg:  after disabling: enable=$enabled, voltage=$voltage"
		fi
	fi

	# This will fail if other consumers have enabled the regulator or if
	# the regulator is marked as always_on.
	if [ $enabled -ne 0 -a $require_disable -gt 0 ]; then
		echo "FAILED: disable $vreg"
		failed=$(($failed + 1))
	fi
}

test_switch_enable() {
	local vreg=$1

	echo 1 > $vreg/enable
	local enabled=`cat $vreg/enable`

	if [ "$verbosity" -gt 0 ]; then
		echo "$vreg:  after enabling: enable=$enabled"
	fi

	if [ $enabled -ne 1 ]; then
		echo "FAILED: enable $vreg"
		failed=$(($failed + 1))
	fi
}

test_switch_disable() {
	local vreg=$1
	local require_disable=$2

	echo 0 > $vreg/enable
	local enabled=`cat $vreg/enable`
	if [ "$verbosity" -gt 0 ]; then
		echo "$vreg:  after disabling: enable=$enabled"
	fi

	# This will fail if other consumers have enabled the regulator or if
	# the regulator is marked as always_on.
	if [ $enabled -ne 0 -a $require_disable -gt 0 ]; then
		echo "FAILED: disable $vreg"
		failed=$(($failed + 1))
	fi
}

test_regulator_enable_disable() {
	local vreg=$1
	local v_low=$2
	local v_high=$3
	local require_disable=$4
	local load_uA=$5

	test_regulator_enable $1 $2 $3 $5
	test_regulator_disable $1 $4 $3 $5
}

test_switch_enable_disable() {
	local vreg=$1
	local require_disable=$2

	test_switch_enable $1
	test_switch_disable $1 $2
}

# test begin

test_iterations=2
verbosity=0
# Parse command line parameters
while [ $# -gt 0 ]; do
        case $1 in
        -n | --nominal)
                # already nominal test case by default
                shift 1
                ;;
        -r | --repeatability)
                test_iterations=10 ; shift 1
                ;;
        -v | --verbosity)
                verbosity=$2 ; shift 2
		;;
        -h | --help | *)
                echo "Usage: $0 [-n | --nominal] [-r | --repeatability] [(-v | --verbosity) <verbosity>]" ;
                exit 1
		;;
        esac
done

# Check target type
if [ $TARGET_TYPE = "7X27" -o $TARGET_TYPE = "7X30" -o $TARGET_TYPE = "8650" -o $TARGET_TYPE = "8650A" ]; then
	VREG_DEV=1
else
	VREG_DEV=0
fi

# Test for devices using vreg driver
if [ "$VREG_DEV" -eq "1" ]; then
	cd $TEST_DEBUG_FS/vreg/ 2>/dev/null

	if [ $? -ne 0 ]; then
		echo "Unable to find $TEST_DEBUG_FS/vreg/"
		echo "Debug fs support required to proceed with tests."
		exit 1;
	fi

	if [ "$verbosity" -gt 0 ]; then
		echo "Testing Regulators"
		echo "=================="
	fi
	failed=0
	vreg="usb"
	mV=2850

	status=`cat $vreg`
	echo  $mV > $vreg
	status=`cat $vreg`
	if [ $status -ne 0 ]; then
		echo "FAILED: vreg_set_level"
		failed=$(($failed + 1))
	fi

	echo  "0" > $vreg
	status=`cat $vreg`
	if [ $status -ne 0 ]; then
		echo "FAILED: vreg_disable"
		failed=$(($failed + 1))
	fi

	echo  "1" > $vreg
	status=`cat $vreg`
	if [ $status -ne 0 ]; then
		echo "FAILED: vreg_enable"
		failed=$(($failed + 1))
	fi

	if [ $failed -eq 0 ]; then
		echo "Regulator test passed"
		exit 0;
	else
		echo "Regulator test failed"
		exit 1;
	fi
fi

# Test for 8660 which utilizes the Linux regulator framework
if [ $TARGET_TYPE = "8660" ]; then
	cd $TEST_DEBUG_FS/regulator/ 2>/dev/null

	if [ $? -ne 0 ]; then
		echo "Unable to find $TEST_DEBUG_FS/regulator/"
		echo "Debug fs support required to proceed with tests."
		exit 1;
	fi

	if [ "$verbosity" -gt 0 ]; then
		echo "Testing Regulators"
		echo "=================="
	fi

	failed=0

	for i in $(seq 1 $test_iterations)
	do
		if [ $test_iterations -gt 1 -a "$verbosity" -gt 0 ]; then
			echo "= Test iteration $i of $test_iterations ="
		fi

		# Test if unused regulators can be turned on and off.
		if [ "$verbosity" -gt 0 ]; then
			echo "-- Unused Regulators --"
		fi
		test_regulator_enable_disable "8058_l19" 2500000 2500000 1 0
		test_regulator_enable_disable "8058_l23" 1200000 1200000 1 0
		test_regulator_enable_disable "8058_l24" 1200000 1200000 1 0
		test_regulator_enable_disable "8901_s2"  1200000 1400000 1 0

		# Test regulators that are used for various purposes, which
		# should be okay to modify.
		if [ "$verbosity" -gt 0 ]; then
			echo "-- Used Regulators --"
		fi
		test_regulator_enable_disable "8058_l10" 2600000 2600000 0 0
		test_regulator_enable_disable "8058_s0"  1150000 1200000 0 0
		test_regulator_enable_disable "8058_ncp" 1800000 1800000 0 0
		test_regulator_enable_disable "8901_l2"  2850000 3300000 0 0
		test_switch_enable_disable "8058_lvs0" 0
		test_switch_enable_disable "8901_mpp0" 0
		test_switch_enable_disable "8901_lvs1" 0
	done

	if [ $failed -gt 0 ]; then
		echo "Regulator test failed"
		exit 1;
	else
		echo "Regulator test passed"
		exit 0;
	fi
fi

# Test for 8960 which utilizes the Linux regulator framework
if [ $TARGET_TYPE = "8960" ]; then
	cd $TEST_DEBUG_FS/regulator/ 2>/dev/null

	if [ $? -ne 0 ]; then
		echo "Unable to find $TEST_DEBUG_FS/regulator/"
		echo "Debug fs support required to proceed with tests."
		exit 1;
	fi

	if [ "$verbosity" -gt 0 ]; then
		echo "Testing Regulators"
		echo "=================="
	fi

	failed=0

	for i in $(seq 1 $test_iterations)
	do
		if [ $test_iterations -gt 1 -a "$verbosity" -gt 0 ]; then
			echo "= Test iteration $i of $test_iterations ="
		fi

		# Test regulators that are used for various purposes, which
		# should be okay to modify.
		test_regulator_enable_disable "8921_s1"  1215000 1235000 0 100000
		test_regulator_enable_disable "8921_s4"  1800000 1800000 0 100000
		test_regulator_enable_disable "8921_l2"  1200000 1200000 0 10000
		test_regulator_enable_disable "8921_l8"  2800000 3000000 0 10000
		test_regulator_enable_disable "8921_l9"  2850000 3000000 0 10000
		test_regulator_enable_disable "8921_l24" 1050000 1200000 0 10000
		test_switch_enable_disable "8921_lvs6" 0
		test_switch_enable_disable "ext_5v" 0
	done

	if [ $failed -gt 0 ]; then
		echo "Regulator test failed"
		exit 1;
	else
		echo "Regulator test passed"
		exit 0;
	fi
fi

# Test for 9615 which utilizes the Linux regulator framework
if [ $TARGET_TYPE = "9615" ]; then
	cd $TEST_DEBUG_FS/regulator/ 2>/dev/null

	if [ $? -ne 0 ]; then
		echo "Unable to find $TEST_DEBUG_FS/regulator/"
		echo "Debug fs support required to proceed with tests."
		exit 1;
	fi

	if [ "$verbosity" -gt 0 ]; then
		echo "Testing Regulators"
		echo "=================="
	fi

	failed=0

	for i in $(seq 1 $test_iterations)
	do
		if [ $test_iterations -gt 1 -a "$verbosity" -gt 0 ]; then
			echo "= Test iteration $i of $test_iterations ="
		fi

		# Test regulators that are used for various purposes, which
		# should be okay to modify.
		test_regulator_enable_disable "8018_s5"  1150000 1350000 0 100000
		test_regulator_enable_disable "8018_s1"  1050000 1250000 0 100000
		test_regulator_enable_disable "8018_l4"  3000000 3075000 0 10000
	done

	if [ $failed -gt 0 ]; then
		echo "Regulator test failed"
		exit 1;
	else
		echo "Regulator test passed"
		exit 0;
	fi
fi

echo "Regulator test failed - unsupported target: $TARGET_TYPE"
exit 1;
