
MODULE_NAME        := DSUTILS
MODULE_CFLAGS      := -DFEATURE_DSUTILS_OFFTARGET
MODULE_ROOT        := ../../../data-noship
MODULE_C_SOURCES   := ds_cmdq.c ds_list.c ds_util.c ds_sl_list.c stm2.c
MODULE_CPP_SOURCES := $(wildcard *.cpp)
MODULE_HEADERS     := $(wildcard *.h)
MODULE_LIB_LOC     := ../lib
MODULE_LIB_TARGET  := libdsutils.so
MODULE_BIN_LOC     :=
MODULE_BIN_TARGET  :=

include $(MODULE_ROOT)/test.mk

