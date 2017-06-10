#-----------------------------------------------------------------------------
# Copyright (c) 2009-2011 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

# Script to test the mpp driver.

TEST_DEBUG_FS="$ANDROID_DATA/debug"

. $TEST_ENV_SETUP
verbosity=0

# Parse command line parameters
while [ $# -gt 0 ]; do
        case $1 in
        -n | --nominal)
                # already nominal test case by default
                shift 1
                ;;
        -v | --verbosity)
                verbosity=$2 ; shift 2
		;;
        -h | --help | *)
                echo "Usage: $0 [-n | --nominal] [(-v | --verbosity) <verbosity>]" ;
                exit 1
		;;
        esac
done

# Check target type
if [ $TARGET_TYPE = "7X27" -o $TARGET_TYPE = "7X30" -o $TARGET_TYPE = "8650" -o $TARGET_TYPE = "8650A" ]; then
	OLD_DEV=1
else
	OLD_DEV=0
fi

# Test for devices using old driver
if [ "$OLD_DEV" -eq "1" ]; then
	cd $TEST_DEBUG_FS/mpp/ 2>/dev/null
	if [ $? -ne 0 ]; then
		echo "Unable to find $TEST_DEBUG_FS/mpp/"
		echo "Debug fs support required to proceed with tests."
		exit 1;
	fi

	if [ "$verbosity" -gt 0 ]; then
		echo "Testing MPP"
		echo "==========="
	fi

	failed=0
	mpp="mpp1"

	status=`cat $mpp`
	level=0
	control=0
	x=65535
	y=1048575
	config=$(( (($level & $x)<<16)|($control & $y) ))
	status=`cat $mpp`
	echo  "$config" > $mpp
	status=`cat $mpp`
	if [ $status -ne 0 ]; then
		echo "FAILED: mpp_config_digital_out"
		failed=$(($failed + 1))
	fi

	if [ $failed -eq 0 ]; then
		echo "MPP test passed"
		exit 0;
	else
		echo "MPP test failed"
		exit 1;
	fi
fi

# Test for newer targets (MSM8660, MSM8960, etc) using new driver (MPP debug output visible through GPIO debugfs)
if [ $TARGET_TYPE = "8660" -o $TARGET_TYPE = "8960" -o $TARGET_TYPE = "8064" -o $TARGET_TYPE = "9615" ]; then
	cd $TEST_DEBUG_FS/ 2>/dev/null
	if [ $? -ne 0 ]; then
		echo "Unable to find $TEST_DEBUG_FS/"
		echo "Debug fs support required to proceed with tests."
		exit 1;
	fi

	if [ "$verbosity" -gt 0 ]; then
		echo "Testing MPP"
		echo "==========="
	fi

	# There should always a constant number of MPPs on a given PMIC chip
	if [ $TARGET_TYPE = "8660" ]; then
		# PMIC 8058 has 12 MPPs
		expected_pmic_mpp_count=12
		driver_name="platform/pm8xxx-mpp"
	elif [ $TARGET_TYPE = "8960" ]; then
		# PMIC 8921 has 12 MPPs
		expected_pmic_mpp_count=12
		driver_name="platform/pm8xxx-mpp"
	elif [ $TARGET_TYPE = "8064" ]; then
		# PMIC 8921 has 12 MPPs
		expected_pmic_mpp_count=12
		driver_name="platform/pm8xxx-mpp"
	elif [ $TARGET_TYPE = "9615" ]; then
		# PMIC 8018 has 6 MPPs
		expected_pmic_mpp_count=6
		driver_name="platform/pm8xxx-mpp"
	else
		echo "MPP test failed - unsupported target: $TARGET_TYPE"
		exit 1;
	fi

	count=0
	in_pmic_mpp_section=0
	failed=0
	line_count=0

	while read line; do
		line_count=$(($line_count + 1))
		if [ $in_pmic_mpp_section -eq 0 ]; then
			in_pmic_mpp_section=`echo $line | $BUSYBOX grep $driver_name | $BUSYBOX wc -l`;
		else
			if [ -z "$line" ]; then
				# encountered a blank line after reading MPP lines; no more MPP entries remain
				in_pmic_mpp_section=0
			else
				# Check if MPP line is well formed
				good=`echo $line | $BUSYBOX grep -E "^gpio-.*(d_in|d_out|bi_dir|a_in|a_out|sink|dtest_sink|dtest_out)" | $BUSYBOX wc -l`;
				if [ "$good" -eq "1" ]; then
					count=$(($count + 1))
				fi
			fi

		fi
	done < gpio

	if [ $count -lt $expected_pmic_mpp_count ]; then
		echo "MPP test failed"
		exit 1;
	else
		echo "MPP test passed"
		exit 0;
	fi
fi

echo "MPP test failed - unsupported target: $TARGET_TYPE"
exit 1;
