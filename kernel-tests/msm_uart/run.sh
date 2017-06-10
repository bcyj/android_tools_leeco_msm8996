#! /bin/sh
set -e
export TEST_ENV_SETUP=test_env_setup.sh
cd `dirname $0` && exec ./msm_uart_test.sh $@
