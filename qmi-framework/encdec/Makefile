CC := gcc

HIDE := @

INCDIR := ../inc
SRCDIR := ./
OBJDIR := ../obj
LIBDIR := ../lib
TARGET := libqmi_encdec.so

CFLAGS := -g
CFLAGS += -O2
CFLAGS += -fno-inline
CFLAGS += -fno-short-enums
CFLAGS += -fPIC

LDFLAGS := -shared -Wl,-soname,$(TARGET)

LIBS :=

INCPATHS := \
	-I. \
	-I$(INCDIR)

SRCDIR_SOURCES  := qmi_idl_accessor.c qmi_idl_lib.c

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
