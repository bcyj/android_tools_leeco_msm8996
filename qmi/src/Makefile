
MODULE_NAME        := QMI
MODULE_CFLAGS      := -DFEATURE_WAIT_FOR_MODEM_HACK
MODULE_ROOT        := ../../data-noship
MODULE_C_SOURCES   := qmi_service.c \
                      qmi.c \
                      qmi_wds_srvc.c \
                      qmi_qos_srvc.c \
                      qmi_nas_srvc.c \
                      qmi_eap_srvc.c \
                      qmi_atcop_srvc.c \
                      qmi_wds_utils.c \
                      qmi_uim_srvc.c \
                      qmi_cat_srvc.c

MODULE_CPP_SOURCES := $(wildcard *.cpp)
MODULE_HEADERS     := $(wildcard *.h)
MODULE_LIB_LOC     := ../lib
MODULE_LIB_TARGET  := libqmi.so
MODULE_BIN_LOC     :=
MODULE_BIN_TARGET  :=

include $(MODULE_ROOT)/test.mk


