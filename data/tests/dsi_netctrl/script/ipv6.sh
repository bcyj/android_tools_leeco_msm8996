#!/system/bin/sh

DSI=./dsi_netctrl_client
RESULT=0

echo "dsi_netctrl_test script: Make a IPv6 data call and verify network connectivity."

if $DSI start &&
   $DSI create call1 &&
   $DSI param call1 ipv 6 &&
   $DSI param call1 umts_profile 5 &&
   $DSI up call1 &&
   DEVICE=`$DSI device call1` ; then

  # The new call came up on $DEVICE.  Set default route to go over this device.
  route add default gw `getprop net.$DEVICE.gw` dev $DEVICE

  # Set the global DNS servers to the ones set on $DEVICE.
  setprop net.dns1 `getprop net.$DEVICE.dns1`
  setprop net.dns2 `getprop net.$DEVICE.dns2`

  # send some data
  if ! /data/busybox/busybox ping6 -c 4 -I $DEVICE ipv6.google.com ; then
    echo "Failed ping"
    RESULT=1
  fi

  # bring down the data call
  if ! $DSI down call1 ; then
    echo "Failed ending data call"
    RESULT=1
  fi
else
  echo "Failed bringing up data call"
  RESULT=1
fi

$DSI stop
exit $RESULT

