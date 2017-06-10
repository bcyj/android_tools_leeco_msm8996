#!/system/bin/sh

. ./dsi_netctrl_common.sh

echo "dsi_netctrl_test script: Make a 1x data call and verify network connectivity."

if ! ($DSI start &&
      $DSI create call1 &&
      $DSI param call1 tech 1x ) ; then
  test_fail "Failed to initialize data call"
fi

# Start and stop the call several times
for i in 1 2 3 4
do
  if ! $DSI up call1 ; then
    test_fail "Failed to bring up call on attempt $i"
  fi

  # Get the name of the device where the call came up.
  if ! DEVICE=$($DSI device call1) ; then
    test_fail "Failed to get device"
  fi

  # Set the DNS settings and try to transfer data on the device.
  setup_network $DEVICE
  data_transfer $DEVICE

  # bring down the data call
  if ! $DSI down call1 ; then
    test_fail "Failed ending data call on attempt $i"
  fi

  # wait for all call cleanup to finish before starting a new call
  sleep 1
done

$DSI release call1
$DSI stop

