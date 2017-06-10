#!/system/bin/sh
echo "on" > /sys/power/state
sleep 1
/data/kernel-tests/fbtest
