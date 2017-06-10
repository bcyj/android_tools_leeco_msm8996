
MODULE_NAME        := QMUXD
MODULE_ROOT        := ../../data-noship
MODULE_CFLAGS      := -DFEATURE_WAIT_FOR_MODEM_HACK -DQMI_DEBUG -DQMI_OFFTARGET
MODULE_C_SOURCES   := qmi_qmux.c \
                      qmi_util.c \
                      linux_qmi_qmux_if_server.c \
                      qmi_platform_qmux_io.c \
                      qmi_platform.c \
                      qmi_platform_config.c

MODULE_CPP_SOURCES :=
MODULE_HEADERS     := $(wildcard *.h)
MODULE_LIB_LOC     := ../lib
MODULE_LIB_TARGET  := libqmuxd.so
MODULE_BIN_LOC     := ../bin
MODULE_BIN_TARGET  := qmuxd

MODULE_LIBDEPS     += ../qmi_client_qmux
MODULE_LIBDEPS     += ../../xmllib/src
MODULE_LIBDEPS     += ../../data-noship/tf
MODULE_LIBDEPS     += ../../data-noship/stubs
MODULE_LIBDEPS     += ../../data/configdb/src
MODULE_LIBDEPS     += ../../data/dsutils/src
MODULE_LIBDEPS     += ../src

MODULE_LOG_FILE    := qmuxd.log
MODULE_PREBUILTS   := ../platform/qmi_config.xml

vpath %.c ../platform
vpath %.c ../src

include $(MODULE_ROOT)/test.mk


