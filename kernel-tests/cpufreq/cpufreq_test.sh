#-----------------------------------------------------------------------------
# Copyright (c) 2008-2012 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

. $TEST_ENV_SETUP

CPUFREQ_DIR="/sys/devices/system/cpu/cpu0/cpufreq"
CMD_CUR_UPTIME="cat /proc/uptime | sed -e 's/\..*//'"

# The following times were chosen after several trials to be as small as
# possible and long enough to allow changes to freq.
SETTLING_TIME=3
RAMP_DOWN_TIME=30
RAMP_UP_TIME=10

if [ ! -d $CPUFREQ_DIR ]; then
	echo "Looks like cpufreq is not supported!"
	echo "Make sure the modules, if any, are loaded."
	exit 1
fi

cd $CPUFREQ_DIR

for gov in ondemand performance powersave
do
	grep $gov < scaling_available_governors > /dev/null
	if [ $? -ne 0 ]; then
		echo "The following governors need to be supported for proper testing:"
		echo "ondemand, performance and powersave."
		exit 1
	fi
done

ORIG_GOV=`cat scaling_governor`
if [ $? -ne 0 ]; then
    echo "Unable to determine orgininal govenor!"
    exit 1
fi

CPU_MAX_FREQ=`cat scaling_max_freq | grep '[0-9]\+'`
if [ $? -ne 0 ]; then
	echo "Unable to determing max freq!"
	exit 1
fi

CPU_MIN_FREQ=`cat scaling_min_freq | grep '[0-9]\+'`
if [ $? -ne 0 ]; then
	echo "Unable to determing min freq!"
	exit 1
fi

if [ $CPU_MIN_FREQ -eq $CPU_MAX_FREQ ]; then
	echo "Max and min freq are same ($CPU_MAX_FREQ KHz)!"
	echo "Test is pointless!"
	exit 1
fi

echo "$ORIG_GOV -> Powersave"
echo powersave > scaling_governor

CPU_CUR_FREQ=`cat scaling_cur_freq | grep '[0-9]\+'`
echo "CPU Frequency in Powersave mode is $CPU_CUR_FREQ KHz"
if [ $CPU_CUR_FREQ -ne $CPU_MIN_FREQ ]; then
	echo "CPU freq is not ramping down to min($CPU_MIN_FREQ KHz)"
	echo "Is $CPU_CUR_FREQ KHz."
	echo "Governor is" $(cat scaling_governor)
	#attempt to restore to orig governor so as not to interfere with future tests
	echo $ORIG_GOV > scaling_governor
	exit 1
fi

echo "Powersave -> Perf"
echo performance > scaling_governor

CPU_CUR_FREQ=`cat scaling_cur_freq | grep '[0-9]\+'`
echo "CPU Frequency in performance mode is $CPU_CUR_FREQ KHz"
if [ $CPU_CUR_FREQ -ne $CPU_MAX_FREQ ]; then
	echo "CPU freq is not ramping up to max($CPU_MAX_FREQ KHz)"
	echo "Is $CPU_CUR_FREQ KHz."
	echo "Governor is" $(cat scaling_governor)
	#attempt to restore to orig governor so as not to interfere with future tests
	echo $ORIG_GOV > scaling_governor
	exit 1
fi


echo "Skipping test of ondemand governor..."
echo "The plan is to create another test for ondemand governor,"
echo "and to create a separate test suite for CPUFreq."

echo "Test passed"

#attempt to restore to orig governor so as not to interfere with future tests
echo $ORIG_GOV > scaling_governor

exit 0

echo "Powersave -> Ondemand"
echo ondemand > scaling_governor

# Give some time for freq to settle
echo "Sleep for ${SETTLING_TIME}s and check freq is at min($CPU_MIN_FREQ KHz)."
sleep $SETTLING_TIME

CPU_CUR_FREQ=`cat scaling_cur_freq | grep '[0-9]\+'`
if [ $CPU_CUR_FREQ -ne $CPU_MIN_FREQ ]; then
	echo "CPU freq is not ramping down to min($CPU_MIN_FREQ KHz)"
	echo "Is $CPU_CUR_FREQ KHz."
	echo "Governor is" $(cat scaling_governor)
	exit 1
fi

CUR_TIME=`cat /proc/uptime | sed -e 's/\..*//'`
if [ $? -ne 0 ]; then
	echo "Unable to determine uptime!"
	exit 1
fi

END_TIME=$(( $CUR_TIME + $RAMP_UP_TIME ))

echo "Looping for ${RAMP_UP_TIME}s and check for freq ramp up."
while [ $CUR_TIME -lt $END_TIME ]
do
	CUR_TIME=`cat /proc/uptime | sed -e 's/\..*//'`
done

CPU_CUR_FREQ=`cat scaling_cur_freq | grep '[0-9]\+'`
# The -ne is important for the next if to be meaningful.
# Using -gt MIN could cause the next if to miss an error case where freq
# stays between MIN and MAX
if [ $CPU_CUR_FREQ -ne $CPU_MAX_FREQ ]; then
	echo "CPU freq is not ramping up to max($CPU_MAX_FREQ KHz)"
	echo "Is $CPU_CUR_FREQ KHz."
	echo "Governor is" $(cat scaling_governor)
	exit 1
fi

# Give some time for freq to settle
echo "Sleep for ${RAMP_DOWN_TIME}s and check freq ramp down."
sleep $RAMP_DOWN_TIME

CPU_CUR_FREQ=`cat scaling_cur_freq | grep '[0-9]\+'`
if [ $CPU_CUR_FREQ -ge $CPU_MAX_FREQ ]; then
	echo "CPU freq is not ramping down to min($CPU_MIN_FREQ KHz)"
	echo "Is $CPU_CUR_FREQ KHz."
	echo "Governor is" $(cat scaling_governor)
	exit 1
fi

echo "Test passed"
