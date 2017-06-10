#! /bin/sh --
set -e
# Uncomment the following line if you need test_env_setup.sh passed
# in via TEST_ENV_SETUP (common for scripts imported from android)
export TEST_ENV_SETUP=test_env_setup.sh
cd `dirname $0`

# Remove/modify the modprobe lines (here and at the end) if your test
# needs fewer/more modules.
modprobe xyz_module -d .

# Replace with proper call to your kernel-test.  Make sure result codes
# will be properly passed back to the caller, and that installed modules
# will be removed in case of error.
./xyzexe $@
es=$?

modprobe -r xyz_module -d .

return $es
