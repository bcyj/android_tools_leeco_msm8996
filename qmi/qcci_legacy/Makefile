CC := gcc

HIDE := @

INCDIR := ../inc
SRCDIR := ../src
PLATDIR := ../platform
OBJDIR := ../obj
BINDIR := ../bin
LIBDIR := ../lib
TARGET := libqcci_legacy.so

CFLAGS := -DFEATURE_DATA_LOG_STDERR
CFLAGS += -fPIC

LDFLAGS := -shared -Wl,-soname,$(TARGET)
LDFLAGS += -Wl,-rpath,../lib

LIBS := -lpthread
LIBS += -lidl
LIBS += -lqmi_client_qmux
LIBS += -lqmi

LIBDIRS := -L../lib

INCPATHS := \
  -I. \
  -I$(INCDIR) \
  -I$(SRCDIR) \
  -I$(PLATDIR) \
  -I../core/lib/inc \
  -I../../common/inc

SOURCES  := qmi_client.c \

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
