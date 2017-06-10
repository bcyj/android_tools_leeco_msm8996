CC := gcc

HIDE := @

SRCDIR := src
OBJDIR := obj
LIBDIR := lib
TARGET := libcsd-client.so

CFLAGS := -g
CFLAGS += -O2
CFLAGS += -fno-inline
CFLAGS += -fno-short-enums
CFLAGS += -fPIC
CFLAGS += -DQCCI_OVER_QMUX

LDFLAGS := -shared -Wl,-soname,$(TARGET)
LDFLAGS += -Wl,-rpath,../../../qmi-framework/lib/
LDFLAGS += -Wl,-rpath,../../../qmi/lib/

LIBS := -lpthread
LIBS += -lqmi_cci
LIBS += -lqmi_common_so

LIBDIRS := -L../../../qmi-framework/lib/

INCPATHS := \
    -I. \
    -I./inc \
    -I../../../qmi-framework/inc \
    -I../../audio-acdb-util/acdb-loader/inc \
    -I../../audio-acdb-util/acdb-mapper/inc \
    -I../../../common/inc \
    -I$(SRCDIR)

SOURCES := core_sound_driver_v01.c csd_client.c

OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

$(LIBDIR)/$(TARGET): $(OBJECTS) Makefile
	@-mkdir -p $(LIBDIR) 2> /dev/null
	$(HIDE) $(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS)

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@-mkdir -p $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIBDIR)/$(TARGET)

all: $(LIBDIR)/$(TARGET)
