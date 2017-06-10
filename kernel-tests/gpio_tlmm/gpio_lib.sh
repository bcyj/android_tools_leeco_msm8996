#-----------------------------------------------------------------------------
# Copyright (c) 2011,2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

# Script to test the gpiolib driver.
. $TEST_ENV_SETUP # TARGET_TYPE filled in

CONFIG="${PWD}/gpio_lib.conf"

verbosity=0

# gpio sysfs path
gpio_path="/sys/class/gpio/gpiochip"

if [ -d /sys/devices/soc0/ ];then
    soc=1
    SOC_DIR="/sys/devices/soc0"
elif [ -d /sys/devices/system/soc/soc0/ ]; then
    soc=1
    SOC_DIR="/sys/devices/system/soc/soc0"
else
    soc=0
fi

# function to get gpio label info given a gpio
# parameters:
# (1) gpio label lists
# (2) position of the label that wants to get
get_label()
{

    if [ "$1" == "" ];then
	label=""
	return 0
    else
	count_label=0
	for j in $1
	do
	    count_label=$(($count_label + 1))
	    if [ "$count_label" = "$2" ]
	    then
		label="$j"
		return 0
	    fi
	done
	echo "label is not found"
	return 1
    fi
}

# function to get ngpio value
# parameters:
# (1) ngpio list
# (2) position of the ngpio that wants to get
get_ngpio()
{
    count_ngpio=0
    for k in $1
    do
	count_ngpio=$(($count_ngpio + 1))
	if [ "$count_ngpio" = "$2" ]
	then
            ngpio="$k"
            return 0
	fi
    done
    echo "ngpio is not found"
    return 1
}

# function to get device type and initialize reference gpio info
# FFA or SURF or FLUID
get_device_type()
{

    device_dmesg_FFA=`dmesg | head -n 8 | grep "FFA"`
    device_dmesg_Surf=`dmesg | head -n 8 | grep "SURF"`
    device_dmesg_Fluid=`dmesg | head -n 8 | grep "FLUID"`

    if [ $soc -eq 1 ];then
	device=`cat $SOC_DIR/hw_platform`
    else
	if [ "$verbosity" -gt 0 ];then
	    echo "soc info doesn't exist"
	fi
	device=""
    fi
    if [ $device = "FFA" ] || [ -n "$device_dmesg_FFA" ]; then
	device_type=FFA
	return 0
    elif [ $device = "Surf" ] || [ -n "$device_dmesg_Surf" ]; then
	device_type=SURF
	return 0
    elif [ $device = "Fluid" ] || [ -n "$device_dmesg_Fluid" ]; then
	device_type=FLUID
	return 0
    else
	return 1
    fi

    return 1
}

# function to get gpio label, base and ngpio for the test target;
# scan each line of the config file to find the corresponding
# gpio label, base and ngpio list for a given target and device.
# if there is no config information for a given target, list
# will stay null.

get_gpio_reference()
{

line_mode="search"

while read rawline
do
    if [ "$rawline" == "" ];then
        continue
    fi

    if [ "$target_name" == "" ]; then
	if [ $soc -eq 1 ];then
            build_id=`cat $SOC_DIR/build_id |
            sed -e 's/^\([0-9]\)\([0-9xX]\)/\1X/'`
            check_name=`echo $build_id | sed -e "s/${rawline}.*//"`

            if [ "$check_name" != "$build_id" ]; then
		target_name=$rawline
		echo "Detected target type: $rawline"
	    else
		continue
            fi
	else
	    echo "can't detect target type"
	    echo "exit test.."
	    exit 1
	fi
    fi

    if [ "$line_mode" == "search" ]; then
        target_match=`echo $rawline | sed -e "s/${target_name}//"`
        if [ "$target_match" == "" ]; then
            line_mode="open_paren"
        fi
        continue
    elif [ "$line_mode" == "open_paren" ]; then
        target_match=`echo $rawline | sed -e 's/\s*{\s*//'`
        if [ "$target_match" == "" ]; then
            line_mode="open"
        fi
        continue
    elif [ "$line_mode" == "open" ]; then
        target_match=`echo $rawline | sed -e 's/\s*}\s*//'`
        if [ "$target_match" == "" ]; then
            line_mode="search"
            continue
        fi
    fi

    rawline=`echo "$rawline" | sed -e 's/[\t]//g'`
    section=`echo "$rawline" | sed -e 's/^\([A-Z_]*\)=[a-zA-Z0-9_ ]*/\1/'`
    raw_payload=`echo "$rawline" | sed -e 's/^[A-Z_]*=\([a-zA-Z0-9 ]*\)/\1/'`

    if [ "$target_name" == "8660" ]; then
	if [ "$device_type" == ""  ]; then
	    get_device_type
	    if [ $? -ne 0 ]; then
		echo "can't find device type"
		echo "test exit..."
		exit 1
	    fi
	fi

	if [ $section = "$device_type"_LABEL ];then
	    gpio_label=$raw_payload
	elif [ $section = "$device_type"_BASE ];then
            gpio_base=$raw_payload
	elif [ $section = "$device_type"_NGPIO ];then
            gpio_ngpio=$raw_payload
	elif [ $section = "$device_type"_INVALID ];then
	    gpio_invalid=$raw_payload
	fi
    else
	if [ $section = "LABEL" ];then
            gpio_label=$raw_payload
        elif [ $section = "BASE" ];then
            gpio_base=$raw_payload
        elif [ $section = "NGPIO" ];then
            gpio_ngpio=$raw_payload
	elif [ $section = "INVALID" ];then
	    gpio_invalid=$raw_payload
        fi
    fi

done < "$config_file"

if [ "$target_name" == "8660" ] ||
   [ "$target_name" == "8960" ] ||
   [ "$target_name" == "9615" ] || [ "$target_name" == "9625" ] ||
   [ "$target_name" == "8974" ] ||
   [ "$target_name" == "8610" ] || [ "$target_name" == "8x10" ] ||
   [ "$target_name" == "8226" ] || [ "$target_name" == "8026" ] || [ "$target_name" == "8x26" ] || [ "$target_name" == "8926" ] ||
   [ "$target_name" == "8084" ] ||
   [ "$target_name" == "8092" ] ||
   [ "$target_name" == "9630" ] ||
   [ "$target_name" == "8916" ] ||
   [ "$target_name" == "8909" ] ||
   [ "$target_name" == "8939" ] ||
   [ "$target_name" == "8936" ] ||
   [ "$target_name" == "8994" ] ||
   [ "$target_name" == "zirc" ] ||
   [ "$target_name" == "8962" ]; then
    if [ "$gpio_label" == "" ] || [ "$gpio_base"  == "" ] || [ "$gpio_ngpio" == "" ] || [ "$gpio_invalid" == "" ]
    then
	return 1
    fi
else
    if [ "$gpio_label" != "" ] || [ "$gpio_base"  == "" ] || [ "$gpio_ngpio" == "" ] || [ "$gpio_invalid" == "" ]
    then
	return 1
    fi
fi

if [ "$verbosity" -gt 0 ]; then
    echo "gpio_labe is $gpio_label"
    echo "gpio_base is $gpio_base"
    echo "gpio_ngpio is $gpio_ngpio"
    echo "gpio_invalid is $gpio_invalid"
fi

return 0
}


# function to test gpio
gpio_valid_test()
{
    echo "reference gpio_label is $gpio_label"
    echo "reference gpio_base is $gpio_base"
    echo "reference gpio_ngpio is $gpio_ngpio"

    count=0
    for i in $gpio_base
    do
	count=$(($count + 1))
	if [ ! -e "$gpio_path""$i" ]
	then
	    echo "$gpio_path$i not found"
	    return 1
	else
	    label_target=`cat "$gpio_path""$i""/label"`
	    if [  $? -ne 0 ]; then
		echo "failed to get label info on target"
		return 1
	    fi
            base_target=`cat "$gpio_path""$i""/base"`
	    if [ $? -ne 0 ]; then
		echo "failed to get base info on target"
                return 1
            fi
            ngpio_target=`cat "$gpio_path""$i""/ngpio"`
	    if [ $? -ne 0 ]; then
		echo "failed to get ngpio info on target"
                return 1
            fi

	    # test base
	    if [ "$base_target" != "$i" ]
	    then
		echo "base_target = $base_target"
		echo "expected base = $i"
		echo "base number is not correct."
		return 1
	    fi

	    # test label
	    get_label "$gpio_label" "$count"
	    if [ $? -ne 0 ]
	    then
		echo "failed to get label"
		return 1
	    fi

	    if [ "$label_target" != "$label" ]
	    then
		echo "label_target = $label_target"
		echo "expected label = $label"
		echo "label is not correct"
		return 1
	    fi

	    # test ngpio
	    get_ngpio "$gpio_ngpio" "$count"
	    if [ $? -ne 0 ]
            then
                echo "failed to get ngpio"
                return 1
            fi

	    if [ "$ngpio_target" != "$ngpio" ]
	    then
		echo "ngpio_target = $ngpio_target"
		echo "expected ngpio = $ngpio"
		echo "ngpio is not correct"
		return 1
	    fi
	fi
    done
    return 0
}

gpio_invalid_test(){
    echo "gpio_invalid is $gpio_invalid"

    for i in $gpio_invalid
    do
	if [ -e "$gpio_path""$i" ];then
            echo "ERROR: $gpio_path$i should not exist"
            return 1
	fi
    done

    return 0
}

# Begin script execution here

linen="0"
target_name="$TARGET_TYPE"

#parse command line argument
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
	-v | --verbosity)
		verbosity=$2 ; shift 2
		;;
	-h | --help | *)
		echo "Usage: $0 [-t <target_name>] [-n] [-a] [-f <config_file>] [-v <verbosity>]" ;
		exit 1
		;;
	esac
done

if [ "$adversarial_test" == "" -a "$nominal_test" == "" ]; then
        nominal_test="exec"
fi

# 7x27 has an ambiguous build_id
# so check as a special case
if [ "$target_name" == "" ]; then
    if [ $soc -eq 1 ];then
	build_id=`cat $SOC_DIR/build_id | sed -e 's/76XXT-.*//'`
	if [ "$build_id" == "" ]; then
            target_name="7X27"
            if [ "$verbosity" -gt 0 ]; then
		echo "Detected target type: $target_name"
            fi
	fi
    fi
fi

# add support to detect 8960
# build id is not avaiable now so look for id info
if [ "$target_name" == "" ]; then
   if [ $soc -eq 1 ];then
	if [ -f "$SOC_DIR/soc_id" ]; then
		id=`cat $SOC_DIR/soc_id`
	else
		id=`cat $SOC_DIR/id`
	fi
	if [ "$id" = "87" ]; then
	    target_name="8960"
	    if [ "$verbosity" -gt 0 ]; then
		echo "Detected target type: $target_name"
            fi
	fi
    fi
fi

# add support to detect 8926
# build id is not avaiable now so look for id info for 8926
if [ "$target_name" == "" ] || [ "$target_name" == "8x26" ]; then
	if [ $soc -eq 1 ];then
		id=`cat /sys/devices/system/soc/soc0/id`
		if [ "$id" = "200" ] || [ "$id" = "224" ]; then
			target_name="8926"
			if [ "$verbosity" -gt 0 ]; then
				echo "Detected target type: $target_name"
			fi
		fi
	fi
fi

if [ "$verbosity" -gt 0 ]; then
    echo "Detected target type: $target_name"
fi

# for new sysfs
if [ -d "/sys/class/gpio/" ];then
    if [ "$config_file" == "" ]; then
        config_file=$CONFIG
    fi

    #get gpio_label gpio_base gpio_ngpi lists
    get_gpio_reference

    if [ $? -ne 0 ]
    then
	echo "not able to get gpio info, target not supported"
	echo "test exit..."
        exit 1
    fi

    if [ "$nominal_test" == "exec" ]; then

	echo "Running Nominal Test"

        #perform valid test
	gpio_valid_test

	if [ $? -eq 0 ]
	then
            echo "Test Passed"
            exit 0
	else
            echo "Test Failed"
            exit 1
	fi
    fi

    if [ "$adversarial_test" == "exec" ]; then

	echo "Running Adversarial Test"

	#perform invalid test
	gpio_invalid_test

	if [ $? -eq 0 ]
	then
	    echo "Test Passed"
	    exit 0
	else
	    echo "Test Failed"
	    exit 1
	fi
    fi
else
    echo "Unable to find /sys/class/gpio/"
    echo "sysfs support required to proceed with tests."
    exit 1;
fi
