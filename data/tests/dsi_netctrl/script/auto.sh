#!/system/bin/sh

. ./dsi_netctrl_common.sh

/* enable ADB logs */
setprop persist.net.logmask "adb"

call_handle=call1

# Perform cleanup and exit the test script
auto_test_fail ()
{
  echo $1
  $DSI stop
  setprop persist.net.logmask "qxdm"
  exit 1
}

echo "dsi_netctrl_test script: Make a data call, automatically determining
 technology, and verify network connectivity."

if ! ($DSI start) ; then
  echo "Failed to initialize data call...trying again"
  sleep 1
  if ! ($DSI start) ; then
    auto_test_fail "Failed to initialize data call"
  fi
fi

if ! ($DSI create $call_handle) ; then
  echo "Failed to create call1..trying again"
  sleep 1
  if ! ($DSI create $call_handle) ; then
    auto_test_fail "Failed to create call1"
  fi
fi

if ! $DSI up $call_handle ; then
  echo "Failed to up call1..trying again"
  sleep 1
  if ! ($DSI create call2) ; then
    auto_test_fail "Failed to create call2"
  fi
  if ! $DSI up call2 ; then
    auto_test_fail "Failed to bring up call2"
  fi
  call_handle=call2
fi

# Get the name of the device where the call came up.
if ! DEVICE=$($DSI device $call_handle) ; then
  echo "Failed to get device name..try again"
  if ! DEVICE=$($DSI device $call_handle) ; then
    auto_test_fail "Failed to get device"
  fi
fi

# Set the DNS settings and try to transfer data on the device.
setup_network $DEVICE
data_transfer $DEVICE

# bring down the data call
if ! $DSI down $call_handle ; then
  echo "Failed to end call1..try again"
  sleep 1
  if ! $DSI down $call_handle ; then
  auto_test_fail "Failed ending data call1"
  fi
fi

$DSI release $call_handle
$DSI stop

setprop persist.net.logmask "qxdm"