# QCRIL Data  makefile
# pre-req:
# from qcril/common location
# mkdir ../obj, ../bin
# from qcril/common/data location
# make
# make test

CC := gcc
HIDE := 
#HIDE := @

INCDIR := ../inc
SRCDIR := ../data
OBJDIR := ../obj
BINDIR := ../bin
TARGET := qcril_data_test

# Enable debug and dependencies
CFLAGS += -g  -MD 
CFLAGS += -DQCRIL_DATA_OFFTARGET -DQCRIL_DATA_TEST -DFEATURE_UNIT_TEST -DFEATURE_DATA_LOG_STDERR
# Linux libc does not support strl* functions.
CFLAGS += -Dstrlcpy=strncpy -Dstrlcat=strncat

LIBS := \
	-lpthread \
	-lrt \
        -ldsutils

LIBDIRS := \
	-L../../../data/dsutils/lib

INCPATHS := \
	-I../../qcril_qmi \
	-I../../../qmi/inc \
	-I../../../qmi/src \
	-I../../../qmi/core/lib/inc \
	-I../../../qmi/services \
	-I../../../qmi/platform \
	-I../../../data/dsi_netctrl/inc \
	-I../../../data/dsutils/inc \
	-I../../../data/netmgr/inc \
	-I../../../data/qdp/inc \
	-I../../../diag/include \
	-I../../../common/inc \
	-I../../.. \
        -I../../../../../../kernel/arch/x86/include \
	-I../../../../../../frameworks/base/include \
	-I../../../../../../hardware/ril/include \
	-I../../../../../../hardware/ril/include/telephony \
	-I../../../../../../system/core/include \

SOURCES := \
           qcril_data_netctrl.c \
           qcril_data_qos.c     \
           qcril_data_utils.c   \
           qcril_data_client.c  \
           qcril_data_stubs.c   \
           qcril_data_test.c

OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

all: $(BINDIR)/$(TARGET)

$(OBJECTS): qcril_data_test.h

$(BINDIR)/$(TARGET): $(OBJECTS)
	@-mkdir $(BINDIR) 2> /dev/null
	$(HIDE) $(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS)

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c Makefile
	@-mkdir $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@  $<

clean:
	\rm -f $(OBJECTS) $(BINDIR)/$(TARGET)

test: $(BINDIR)/$(TARGET)
	$(BINDIR)/$(TARGET) 

