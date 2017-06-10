
MODULE_NAME        := QMICL_QMUX
MODULE_ROOT        := ../../data-noship
MODULE_CFLAGS      := -DFEATURE_WAIT_FOR_MODEM_HACK
MODULE_C_SOURCES   := linux_qmi_qmux_if_client.c \
                      qmi_platform.c \
                      qmi_qmux_if.c \
                      qmi_util.c
MODULE_CPP_SOURCES :=
MODULE_HEADERS     := $(wildcard *.h)
MODULE_LIB_LOC     := ../lib
MODULE_LIB_TARGET  := libqmi_client_qmux.so
MODULE_BIN_LOC     :=
MODULE_BIN_TARGET  :=

vpath %.c ../platform
vpath %.c ../src

include $(MODULE_ROOT)/test.mk


