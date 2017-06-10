DSI=./dsi_netctrl_client

BUSYBOX=/data/busybox/busybox

PROFILE_TECH_3GPP=1
PROFILE_TECH_3GPP2=0
PROFILE_TECH_AUTO=2
PROFILE_AUTH_PAP_CHAP_NOT_ALLOWED=0
PROFILE_AUTH_PAP_ONLY_ALLOWED=1
PROFILE_AUTH_CHAP_ONLY_ALLOWED=2
PROFILE_AUTH_PAP_CHAP_BOTH_ALLOWED=3
PROFILE_IP_4=0
PROFILE_IP_6=1

# End always on data call. Sleep to ensure it has ended before continuing.
echo -n "1" > /data/dun
sleep 1

# Perform cleanup and exit the test script
test_fail ()
{
  echo $1
  $DSI stop
  exit 1
}

# Configure the network so data can be sent/received over call $1.
#   - Set the default route to the IP routing table
#   - Copy the primary and secondary DNS addresses from the device
#     specific properties to the default DNS properties.
setup_network ()
{
  route add default gw `getprop net.$1.gw` dev $1
  setprop net.dns1 `getprop net.$1.dns1`
  setprop net.dns2 `getprop net.$1.dns2`
}

# Attempt to perform data transfer on a specific device
data_transfer ()
{
  if ! ping -c 4 -I $1 qualnet.qualcomm.com ; then
    test_fail "Ping data transfer test failed"
  fi
}

# parse the output of profile lookup and return the 3gpp profile.
3gpp_profile ()
{
  echo ${1%%[[:space:]][[:digit:]]*}
}

# parse the output of profile lookup and return the 3gpp2 profile.
3gpp2_profile ()
{
  echo ${1##[[:digit:]]*[[:space:]]}
}
