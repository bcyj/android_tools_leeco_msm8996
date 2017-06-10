CC := gcc

HIDE := @

INCDIR := ../inc
SRCDIR := .
OBJDIR := ../../../obj
LIBDIR := ../../../lib
TARGET := libidl.so

CFLAGS := -DFEATURE_DATA_LOG_STDERR
CFLAGS += -fPIC

LDFLAGS := -shared -Wl,-soname,$(TARGET)

LIBS := -lpthread

INCPATHS := \
  -I. \
  -I$(INCDIR) \
  -I$(SRCDIR) \
  -I../../common/inc

SOURCES  := qmi_idl_lib.c \
            qmi_idl_accessor.c

OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

$(LIBDIR)/$(TARGET): $(OBJECTS) Makefile
	@-mkdir -p $(LIBDIR) 2> /dev/null
	$(HIDE) $(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBDIRS) $(LIBS)

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c
	@-mkdir -p $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIBDIR)/$(TARGET)

all: $(LIBDIR)/$(TARGET)
