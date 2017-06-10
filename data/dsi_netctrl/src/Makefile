
MODULE_NAME        := DSI
MODULE_ROOT        := ../../../data-noship
MODULE_CFLAGS      := -DDSI_NETCTRL_OFFTARGET -DFEATURE_DSI_MULTIMODEM_ROUTELOOKUP -DFEATURE_DSI_TEST
MODULE_C_SOURCES   := $(wildcard *.c)
MODULE_CPP_SOURCES := $(wildcard *.cpp)
MODULE_HEADERS     := $(wildcard *.h)
MODULE_LIB_LOC     := ../lib
MODULE_LIB_TARGET  := libdsi_netctrl.so
MODULE_BIN_LOC     :=
MODULE_BIN_TARGET  :=

MODULE_PREBUILTS   := dsi_config.xml

include $(MODULE_ROOT)/test.mk

