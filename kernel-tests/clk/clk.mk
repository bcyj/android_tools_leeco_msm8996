################################################################################
# @file  tests/unit/drivers/clk/Makefile
# @brief Makefile for the Qualcomm MSM clock driver unit test framework.
################################################################################

ROOT := ../../../../..

include $(ROOT)/LINUX/build/makefiles/default.mk

APP_NAME := clk_test.sh

INSTALL_DIR := $(RAMDISK_QCT_TSTDIR)

SCRIPTS	:= clk_test.sh

DDTF_PKG_VER := $(APP_NAME)_0.1
DDTF_PKG_FILES += $(SCRIPTS)

include $(ROOT)/LINUX/build/makefiles/app.mk
