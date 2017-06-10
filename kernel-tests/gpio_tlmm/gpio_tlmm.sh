#-----------------------------------------------------------------------------
# Copyright (c) 2009, 2013 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

# Script to test the gpio driver.

CONFIG="${PWD}/gpio_tlmm.conf"
TEST_DEBUG_FS="$ANDROID_DATA/debug"

verbosity=0
failed=0

func=0
output=1
pull=0
drvstr=0

pack_gpio(){
	raw_config=$(( ((($1 & 1023)<<4)) | \
				($2 & 15) | \
				(($3 & 1) << 14) | \
				(($4 & 3) << 15) | \
				(($5 & 15) << 17) ))
	echo $raw_config
}

######################
# valid gpio test case
######################
valid_test() {
	for gpio_num in $valid_list
	do
		if [ "$verbosity" -gt 1 ]; then
			echo "valid_test: enable $gpio_num"
		fi
		raw_config=`pack_gpio $gpio_num $func $output $pull $drvstr`
		status=`cat enable`
		echo "$raw_config" > enable
		status=`cat enable`
		if [ $status -ne 0 ]; then
			echo "ERROR: Enabling Valid GPIO $gpio_num"
			failed=$(($failed + 1))
		fi
		if [ "$verbosity" -gt 1 ]; then
			echo "valid_test: disable $gpio_num"
		fi
		status=`cat disable`
		echo "$raw_config" > disable
		status=`cat disable`
		if [ $status -ne 0 ]; then
			echo "ERROR: Disabling Valid GPIO $gpio_num"
			failed=$(($failed + 1))
		fi
	done

	return $failed
}

########################
# invalid gpio test case
########################
invalid_test() {
	for gpio_num in $invalid_list
	do
		if [ "$verbosity" -gt 1 ]; then
			echo "invalid_test: enable $gpio_num"
		fi
		raw_config=`pack_gpio $gpio_num $func $output $pull $drvstr`
		status=`cat enable`
		echo "$raw_config" > enable
		status=`cat enable`
		if [ $status -eq 0 ]; then
			echo "ERROR: Enabling invalid GPIO $gpio_num did not fail as expected"
			failed=$(($failed + 1))
		fi
	done

	return $failed
}

############################
# non-remoted gpio test case
############################
normt_test() {
	for gpio_num in $normt_list
	do
		if [ "$verbosity" -gt 1 ]; then
			echo "normt_test: enable $gpio_num"
		fi
		raw_config=`pack_gpio $gpio_num $func $output $pull $drvstr`
		status=`cat enable`
		echo "$raw_config" > enable
		status=`cat enable`
		if [ $status -eq 0 ]; then
			echo "ERROR: Enabling non-remoted GPIO $gpio_num did not fail as expected"
			failed=$(($failed + 1))
		fi
	done

	return $failed
}

# Begin script execution here

linen="0"
target_name=""

while [ $# -gt 0 ]; do
	case $1 in
	-f | --config)
		config_file=$2 ; shift 2
		;;
	-t | --target)
		target_name=$2 ; shift 2
		;;
	-n | --nominal)
		nominal_test="exec" ; shift 1
		;;
	-a | --adversarial)
		adversarial_test="exec" ; shift 1
		;;
	-s | -stress)
		stress_test="exec" ; shift 1
		;;
	-r | --repeatability)
		repeatability_test="exec" ; shift 1
		;;
	-v | --verbosity)
		verbosity=$2 ; shift 2
		;;
	 -h | --help | *)
		echo "Usage: $0 [-t <target_name>] [-n] [-a] [-s] [-r] [-v <verbosity>" ;
		exit 1
		;;
	esac
done

if [ "$adversarial_test" == "" -a "$stress_test" == "" -a "$repeatability_test" == "" ]; then
	nominal_test="exec"
fi

if [ "$config_file" == "" ]; then
	config_file=$CONFIG
fi

cd $TEST_DEBUG_FS/gpio/ 2>/dev/null
if [ $? -ne 0 ]; then
       echo "Unable to find /$TEST_DEBUG_FS/gpio/"
       echo "Debug fs support required to proceed with tests."
       exit 1;
fi

# 7x27 has an ambiguous build_id
# so check as a special case
if [ "$target_name" == "" ]; then
	if [ -f /sys/devices/soc0/build_id ]; then
		build_id=`cat /sys/devices/soc0/build_id | sed -e 's/76XXT-.*//'`
	else
		build_id=`cat /sys/devices/system/soc/soc0/build_id | sed -e 's/76XXT-.*//'`
	fi
	if [ "$build_id" == "" ]; then
		target_name="7X27"
		if [ "$verbosity" -gt 0 ]; then
			echo "Detected target type: $target_name"
		fi
	fi
fi

line_mode="search"

while read rawline; do
	linen=$(($linen + 1))

	if [ "$rawline" == "" ]; then
		continue
	fi

	if [ "$target_name" == "" ]; then
		if [ -f /sys/devices/soc0/build_id ]; then
			build_id=`cat /sys/devices/soc0/build_id |
				  sed -e 's/^\([0-9]\)\([0-9xX]\)/\1X/'`
		else
			build_id=`cat /sys/devices/system/soc/soc0/build_id |
				  sed -e 's/^\([0-9]\)\([0-9xX]\)/\1X/'`
		fi
		check_name=`echo $build_id | sed -e "s/${rawline}.*//"`
		if [ "$check_name" != "$build_id" ]; then
			target_name=$rawline
			if [ "$verbosity" -gt 0 ]; then
				echo "Detected target type: $rawline"
			fi
		else
			continue
		fi
	fi

	if [ "$line_mode" == "search" ]; then
		target_match=`echo $rawline |sed -e "s/${target_name}//"`
		if [ "$target_match" == "" ]; then
			line_mode="open_paren"
			continue
		fi
		continue
	elif [ "$line_mode" == "open_paren" ]; then
		target_match=`echo $rawline |sed -e 's/\s*{\s*//'`
		if [ "$target_match" == "" ]; then
			line_mode="open"
			continue
		fi
		continue
	elif [ "$line_mode" == "open" ]; then
		target_match=`echo $rawline |sed -e 's/\s*}\s*//'`
		if [ "$target_match" == "" ]; then
			line_mode="search"
			continue
		fi
	fi

	rawline=`echo "$rawline" | sed -e 's/[\t ]//g'`
	syntax=`echo "$rawline" |sed -e 's/^\([A-Z_]*\)=[0-9,-]*//'`

	if [ "$syntax" != "" ]; then
		echo "Syntax error in config $CONFIG on line $linen"
		exit 1
	fi

	section=`echo "$rawline" |sed -e 's/^\([A-Z_]*\)=[0-9,-]*/\1/'`
	raw_payload=`echo "$rawline" |sed -e 's/^[A-Z_]*=\([0-9,-]*\)/\1/'`
	section_range=`echo $raw_payload | awk '

	# Begin AWK script
	# input example: 1,2,3-10,11,15
	# output example: 1 2 3 4 5 6 7 8 9 10 11 15
	BEGIN {
		FS=","
	}
	{
		for (i=1; i<=NF; i++) {
			if ($i ~ /[0-9]*-[0-9]*/) {
				ndx = index($i, "-")
				min = substr($i,0,ndx-1) + 0
				max = substr($i,ndx+1,length($i)) + 0
				if (max > min)
					for (j=min; j<=max; j++)
						printf "%u ", j
			}
			else
				printf "%s ", $i
		}
	}
	END {
		printf "\n"
	}
	# End AWK script'`

	if [ $section == "VALID" ]; then
		valid_list=$section_range
	elif [ $section == "INVALID" ]; then
		invalid_list=$section_range
	elif [ $section == "NON_RMT" ]; then
		normt_list=$section_range
	else
		echo "Invalid section in config $CONFIG on line $linen"
		exit 1

	fi

done < "$CONFIG"

if [ "$nominal_test" == "exec" ]; then
	if [ "$valid_list" != "" ]; then
		if [ "$verbosity" -gt 0 ]; then
			echo "Running Nominal Test"
		fi
		valid_test
	else
		echo "Insufficient config data to run Nominal test"
		exit 1
	fi
fi

if [ "$adversarial_test" == "exec" ]; then
	if [ "$invalid_list" != "" -o "$normt_list" != "" ]; then
		if [ "$verbosity" -gt 0 ]; then
			echo "Running Adversarial Test"
		fi
		if [ "$invalid_list" != "" ]; then
			invalid_test
		fi
		if [ "$normt_list" != "" ]; then
			normt_test
		fi
	else
		echo "Insufficient config data to run Adversarial test"
		exit 1
	fi
fi

if [ "$repeatability_test" == "exec" ]; then
	if [ "$valid_list" != "" ]; then
		if [ "$verbosity" -gt 0 ]; then
			echo "Running Repeatability Test"
		fi

		for i in $(seq 1 10)
		do
			valid_test
		done
	else
		echo "Insufficient config data to run Repeatability test"
		exit 1
	fi

fi

if [ "$stress_test" == "exec" ]; then
	if [ "$nominal_list" != "" -o "$invalid_list" != "" -o "$normt_list" != "" ]; then
		if [ "$verbosity" -gt 0 ]; then
			echo "Running Stress Test"
		fi

		for i in $(seq 1 100)
		do
			if [ "$normt_list" != "" ]; then
				normt_test
			fi
			if [ "$invalid_list" != "" ]; then
				invalid_test

			fi
			if [ "$valid_list" != "" ]; then
				valid_test

			fi
		done
	else
		echo "Insufficient config data to run Stress test"
		exit 1

	fi
fi


if [ $failed -gt 0 ]; then
	echo "Test failed"
	exit 1
else
	echo "Test passed"
	exit 0
fi
