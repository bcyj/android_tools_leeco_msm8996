#-----------------------------------------------------------------------------
# Copyright (c) 2011-2013,2015 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

. $TEST_ENV_SETUP

CPUHOTPLUG_DIR="/sys/devices/system/cpu"

SYS_PM_8x60="/sys/module/pm_8x60/"
SLEEP_MODE_NODE_STD_8x60_1="modes/cpu1/standalone_power_collapse/suspend_enabled"

if [ $TEST_TARGET == "ANDROID" ]
then
    RMMOD="/system/bin/rmmod"
    INSMOD="/system/bin/insmod"
else
    RMMOD="rmmod"
    INSMOD="insmod"
fi
CORECTL_MODULE="/system/lib/modules/core_ctl.ko"
CORECTL_PRESENT=$(lsmod | grep core_ctl)

get_num_cpu(){
num_cpu=`ls $CPUHOTPLUG_DIR | grep "cpu[0-9]" | wc -l`
}

# Function flip_value
# Parameters:
# 1) cpu index
# 2) new online value
# return 0 if sucessfully change online to new online value
# - otherwise, return 1
flip_value(){
    echo $2 > "$CPUHOTPLUG_DIR/cpu$i/online"
    if [ `cat $CPUHOTPLUG_DIR/cpu$1/online` -ne $2 ]; then
	echo "flip online value for cpu$i failed"
	return 1
    fi
    return 0
}

# Function test_cpu_offline
# Parameters:
# 1) cpu index
# 2) online value
# return 0 on success otherwise return 1
test_cpu_offline(){

offline_info=`cat /sys/devices/system/cpu/offline`
if [ "$offline_info" = "" ];then offline_info="X"
fi
cpu_info=`echo $1 | awk '/['$offline_info']/ {print}'`

if [ $2 -eq 0 ]
then
    if [ "$cpu_info" = "" ];then
	echo "ERROR: cpu$1 not present in /sys/devices/system/cpu/offline"
	return 1
    fi
else
    if [ "$cpu_info" != "" ];then
	echo "ERROR: cpu$1 present in /sys/devices/system/cpu/offline"
	return 1
    fi
fi

return 0
}

# Function test_interrupts
# Parameters:
# 1) cpu index
# 2) online value
# return 0 on success otherwise return 1
test_interrupts(){

interrupt_col=$(( 2 + $1 ))

if [ $verbosity -gt 0 ];then
echo "interrput_col is $interrupt_col"
fi

if [ $2 -eq 0 ];then
    cpu_match=`cat /proc/interrupts | awk '/'CPU$1'/ {print}'`
    if [ "$cpu_match" != "" ];then
	echo "ERROR: cpu$1 is not offline"
	return 1
    fi
else
    cpu_irs_1=`cat /proc/interrupts | awk '{print $'$interrupt_col'}'`

    sleep 5

    cpu_irs_2=`cat /proc/interrupts | awk '{print $'$interrupt_col'}'`

    if [ "$cpu_irs_1" = "$cpu_irs_2" ];then
	echo "ERROR: cpu$1 is not receiving irq when it is online"
	return 1
    fi
fi

return 0
}

save_disable_core_ctl() {
    for i in $(seq 0 $num_cpu_test)
    do
        if [ -d $CPUHOTPLUG_DIR/cpu$i/core_ctl ]; then
            CORECTLDIR=$CPUHOTPLUG_DIR/cpu$i/core_ctl
            BUSY_DOWN_THRES[i]=$(cat $CORECTLDIR/busy_down_thres)
            BUSY_UP_THRES[i]=$(cat $CORECTLDIR/busy_up_thres)
            IS_BIG_CLUSTER[i]=$(cat $CORECTLDIR/is_big_cluster)
            MAX_CPUS[i]=$(cat $CORECTLDIR/max_cpus)
            MIN_CPUS[i]=$(cat $CORECTLDIR/min_cpus)
            OFFLINE_DELAY_MS[i]=$(cat $CORECTLDIR/offline_delay_ms)
            TASK_THRES[i]=$(cat $CORECTLDIR/task_thres)
        fi
    done
    RQ_AVG_PERIOD_MS=$(cat /sys/module/core_ctl/parameters/rq_avg_period_ms)

    $RMMOD core_ctl

    # Put all CPUs back online
    for i in $(seq 0 $num_cpu_test)
    do
        echo 1 > $CPUHOTPLUG_DIR/cpu$i/online
    done
}

enable_restore_core_ctl() {
    $INSMOD $CORECTL_MODULE
    for i in $(seq 0 $num_cpu_test)
    do
        if [ -d $CPUHOTPLUG_DIR/cpu$i/core_ctl ]; then
            CORECTLDIR=$CPUHOTPLUG_DIR/cpu$i/core_ctl
            echo ${BUSY_DOWN_THRES[$i]} > $CORECTLDIR/busy_down_thres
            echo ${BUSY_UP_THRES[$i]} > $CORECTLDIR/busy_up_thres
            echo ${IS_BIG_CLUSTER[$i]} > $CORECTLDIR/is_big_cluster
            echo ${MIN_CPUS[$i]} > $CORECTLDIR/min_cpus
            echo ${MAX_CPUS[$i]} > $CORECTLDIR/max_cpus
            echo ${OFFLINE_DELAY_MS[$i]} > $CORECTLDIR/offline_delay_ms
            echo ${TASK_THRES[$i]} > $CORECTLDIR/task_thres
        fi
    done
    echo $RQ_AVG_PERIOD_MS > /sys/module/core_ctl/parameters/rq_avg_period_ms
}

do_test(){

get_num_cpu

if [ $verbosity -gt 0 ];then
    echo "num_cpu is $num_cpu"
fi

if [ $num_cpu -le 1 ];then
    echo "WARN: Test only supported on SMP system"
    return 0
fi

num_cpu_test=$(($num_cpu - 1))

if [ -e $SYS_PM_8x60/$SLEEP_MODE_NODE_STD_8x60_1 ]; then
    # need to enable standalone power collapse for cpuhotplug to work
    echo "enable standalone PC: $SYS_PM_8x60/$SLEEP_MODE_NODE_STD_8x60_1"
    echo 1 > $SYS_PM_8x60/$SLEEP_MODE_NODE_STD_8x60_1
fi

if [ -n "$CORECTL_PRESENT" ]; then
    if [ $verbosity -gt 0 ];then
        echo "Core control module detected. Saving and disabling core control..."
    fi
    save_disable_core_ctl
fi

for i in $(seq 1 $num_cpu_test)
do
    old_online=`cat $CPUHOTPLUG_DIR/cpu$i/online`
    new_online=$(( ! $old_online  ))

    if [ $verbosity -gt 0 ];then
        echo "Testing CPU$i..."
    fi

    if [ $verbosity -gt 0 ];then
	echo "old online is $old_online"
	echo "new online is $new_online"
    fi

    flip_value $i $new_online

    if [ $? -ne 0 ]; then
	return 1
    fi

    test_cpu_offline $i $new_online

    if [ $? -ne 0 ]; then
	#flip online value back
	flip_value $i $old_online
	return 1
    fi

    test_interrupts $i $new_online

    if [ $? -ne 0 ]; then
	#flip online value back
	flip_value $i $old_online
	return 1
    fi

    #flip online value back
    flip_value $i $old_online

    if [ $? -ne 0 ]; then
	return 1
    fi

    test_cpu_offline $i $old_online

    if [ $? -ne 0 ]; then
	return 1
    fi

    test_interrupts $i $old_online

    if [ $? -ne 0 ]; then
	return 1
    fi
done

if [ -n "$CORECTL_PRESENT" ]; then
    if [ $verbosity -gt 0 ];then
        echo "Restoring core control"
    fi
    enable_restore_core_ctl
fi

return 0
}

# Begin script execution here

nominal_test=0
repeatability_test=0
verbosity=0

while [ $# -gt 0 ]
do
    case $1 in
    -n | --nominal)
    nominal_test=1
    shift 1
    ;;
    -r | --repeatability)
    repeatability_test=1
    shift 1
    ;;
    -v | --verbosity)
    verbosity=$2
    shift 2
    ;;
    -h | --help | *)
    echo "Usage: $0 [-n] [-s] [-v <verbosity>]"
    exit 1
    ;;
    esac
done

if ! [ -d $CPUHOTPLUG_DIR ]; then
    echo "ERROR: $CPUHOTPLUG_DIR is not a directory"
    exit 1
fi

if [ $nominal_test -eq 0 -a $repeatability_test -eq 0 ]; then
    nominal_test=1
fi

#do nominal test
if [ $nominal_test -eq 1 ];then
    if [ $verbosity -gt 0 ];then
	echo "=== Running Nominal Test ==="
    fi
    do_test
    if [ $? -eq 0 ];then
	echo "Nominal Test Passed"
    else
	echo "Nominal Test Failed"
	exit 1
    fi
fi

#do repeatability test
if [ $repeatability_test -eq 1 ];then
    if [ $verbosity -gt 0 ];then
	echo "=== Running Repeatability Test ==="
    fi
    for i in $(seq 1 20)
    do
	do_test
	if [ $? -ne 0  ];then
	    echo "Repeatability Test Failed"
	    exit 1
	fi
    done
    echo "Repeatability Test Passed"
fi
