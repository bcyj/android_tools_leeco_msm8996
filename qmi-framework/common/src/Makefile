CC := gcc

HIDE := @

INCDIR := ../inc
SRCDIR := ../src
OBJDIR := ../../obj
BINDIR := ../../bin
LIBDIR := ../../lib
TARGET := libqmi_common_so.so

CFLAGS := -g
CFLAGS += -O2
CFLAGS += -fno-inline
CFLAGS += -fno-short-enums
CFLAGS += -fPIC

LDFLAGS := -shared -Wl,-soname,$(TARGET)

LIBS := -lpthread

INCPATHS := \
	-I. \
	-I$(INCDIR) \
	-I../../inc

SRCDIR_SOURCES  := common_v01.c

SRCDIR_OBJS := $(addprefix $(OBJDIR)/, $(SRCDIR_SOURCES:.c=.o))
OBJECTS := $(SRCDIR_OBJS)

$(LIBDIR)/$(TARGET): $(SRCDIR_OBJS) Makefile
	@-mkdir -p $(LIBDIR) 2> /dev/null
	$(HIDE) $(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(SRCDIR_OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@-mkdir -p $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIBDIR)/$(TARGET)

all: $(LIBDIR)/$(TARGET)
