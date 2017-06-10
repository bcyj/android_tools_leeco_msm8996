CC := gcc

HIDE := @

INCDIR := ../inc
SRCDIR := ../src
COMMDIR := ../../common/src
SMEMLOGDIR := ../../smem_log
OBJDIR := ../../obj
LIBDIR := ../../lib
TARGET := libqmi_cci.so

CFLAGS := -g
CFLAGS += -O2
CFLAGS += -fno-inline
CFLAGS += -fno-short-enums
CFLAGS += -fPIC
CFLAGS += -DQCCI_OVER_QMUX

LDFLAGS := -shared -Wl,-soname,$(TARGET)

LIBS := -lpthread
LIBS += -lqmi_encdec
LIBS += -lqmi_client_qmux

LIBDIRS := -L../../lib
LIBDIRS += -L../../../qmi/lib

INCPATHS := \
	-I. \
	-I$(INCDIR) \
	-I../../inc \
	-I../../common/inc \
        -I../../smem_log \
	-I../../../qmi/inc \
	-I../../../qmi/src \
	-I../../../qmi/platform \
	-I../../../common/inc

COMMDIR_SOURCES := common_v01.c

SRCDIR_SOURCES  := qmi_cci_common.c \
                   qmi_cci_target.c \
                   qmi_cci_xport_ipc_router.c \
                   qmi_cci_xport_qmuxd.c

SMEMLOGDIR_SOURCES  := smem_log.c

COMMDIR_OBJS := $(addprefix $(OBJDIR)/, $(COMMDIR_SOURCES:.c=.o))
SMEMLOGDIR_OBJS := $(addprefix $(OBJDIR)/, $(SMEMLOGDIR_SOURCES:.c=.o))
SRCDIR_OBJS := $(addprefix $(OBJDIR)/, $(SRCDIR_SOURCES:.c=.o))
OBJECTS := $(COMMDIR_OBJS) \
           $(SMEMLOGDIR_OBJS) \
           $(SRCDIR_OBJS)

$(LIBDIR)/$(TARGET): $(COMMDIR_OBJS) $(SMEMLOGDIR_OBJS) $(SRCDIR_OBJS) Makefile
	@-mkdir -p $(LIBDIR) 2> /dev/null
	$(HIDE) $(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS)

$(COMMDIR_OBJS): $(OBJDIR)/%.o: $(COMMDIR)/%.c
	@-mkdir -p $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@ $<

$(SMEMLOGDIR_OBJS): $(OBJDIR)/%.o: $(SMEMLOGDIR)/%.c
	@-mkdir -p $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@ $<

$(SRCDIR_OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@-mkdir -p $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIBDIR)/$(TARGET)

all: $(LIBDIR)/$(TARGET)
