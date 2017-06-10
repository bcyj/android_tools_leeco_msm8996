#! /bin/sh --
set -e
export TEST_ENV_SETUP=test_env_setup.sh
cd `dirname $0` && exec ./timertest.sh $1
