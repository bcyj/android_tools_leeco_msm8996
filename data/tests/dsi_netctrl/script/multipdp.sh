#!/system/bin/sh

set -x

. ./dsi_netctrl_common.sh

echo "dsi_netctrl_test script: Make data calls on multiple PDP contexts."

# start a call using a new profile. Parameter is used to uniquely identify call and profile names.
start_call ()
{
  if $DSI profile create profile$1 &&
     $DSI profile set profile$1 tech $PROFILE_TECH_3GPP &&
     $DSI profile set profile$1 apn apn$1 &&
     PROFILE=$( $DSI profile lookup profile$1 ) ; then
    if ! ($DSI create call$1 &&
          $DSI param call$1 umts_profile `3gpp_profile $PROFILE` &&
          $DSI up call$1 ) ; then
      test_fail "Failed to start call"
    fi
  else
    test_fail "Failed to create a new profile"
  fi
  return 0
}

# Start the daemon
if ! $DSI start ; then
  test_fail "failed to start daemon"
fi

# Bring up a call on default profile
if ! ( $DSI create call0 &&
       $DSI up call0 ) ; then
  test_fail "failed to bring up a call"
fi

# Get the name of the device where the call came up.
if ! DEVICE=$($DSI device call0) ; then
  test_fail "Failed to get device"
fi

# Set the DNS settings and try to transfer data on the device.
setup_network $DEVICE
data_transfer $DEVICE

# if the technology is UMTS, then try to bring up multiple pdp calls
if ! TECH=$( $DSI tech call0 ) ; then
  test_fail "Failed to get technology"
fi

echo "Technology string is \"$TECH\""

# Check if the technology string starts with "UMTS".
if ! $BUSYBOX expr "$TECH" : "UMTS.*" ; then
  echo "Technology is not UTMS.  Pass test without attempting multipdp calls."
  $DSI stop
  set +x
  exit 0
fi

if ! $DSI profile init $DEVICE ; then
  test_fail "profile init failed"
fi

# start two more calls using new pdp profiles
for i in 1 2
do
  start_call $i
done

# bring down the data calls
for i in 2 1 0
do
  if ! $DSI down call$i ; then
    test_fail "Failed ending data call$i"
  fi
done

# release profiles
for i in 1 2
do
  if ! $DSI profile release profile$i ; then
    test_fail "failed to release profile"
  fi
done

if ! $DSI profile deinit ; then
  test_fail "failed to deinitialize QDP"
fi

$DSI stop
set +x
exit 0


