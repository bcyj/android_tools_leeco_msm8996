CC := gcc
HIDE := @

INCDIR := ../inc
SRCDIR := .
PLATDIR := ../platform
OBJDIR := ../obj
BINDIR := ../bin
LIBDIR := ../lib
TARGET := libqmiservices.so

LDFLAGS := -shared -Wl,-soname,$(TARGET)
LDFLAGS += -Wl,-rpath,../lib

CFLAGS += -DFEATURE_WAIT_FOR_MODEM_HACK
CFLAGS += -DFEATURE_DATA_LOG_STDERR
CFLAGS += -fPIC

LIBS := -lidl

LIBDIRS := -L../lib

INCPATHS := \
  -I. \
  -I$(INCDIR) \
  -I$(SRCDIR) \
  -I$(PLATDIR) \
  -I../core/lib/inc \
  -I../../common/inc \

SOURCES := \
  common_v01.c \
  voice_service_common_v02.c \
  voice_service_v02.c \
  wireless_data_service_v01.c \
  wireless_messaging_service_v01.c \
  device_management_service_v01.c \
  network_access_service_v01.c \
  user_identity_module_v01.c \
  card_application_toolkit_v02.c \
  phonebook_manager_service_v01.c \
  sar_vs_service_v01.c \
  specific_absorption_rate_v01.c \
  control_service_v01.c \
  qmi_ims_vt_v01.c \
  qualcomm_mobile_access_point_v01.c \
  circuit_switched_video_telephony_v01.c \
  ip_multimedia_subsystem_video_telephony_v01.c \
  ip_multimedia_subsystem_presence_v01.c \
  coexistence_manager_v01.c \
  ip_multimedia_subsystem_settings_v01.c \
  radio_frequency_radiated_performance_enhancement_v01.c \
  persistent_device_configuration_v01.c

OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

$(LIBDIR)/$(TARGET): $(OBJECTS)
	@-mkdir -p $(LIBDIR) 2> /dev/null
	$(HIDE) $(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBDIRS) $(LIBS)

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c Makefile
	@-mkdir -p $(OBJDIR) 2> /dev/null
	$(HIDE) $(CC) -c $(CFLAGS) $(INCPATHS) -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIBDIR)/$(TARGET)

all: $(LIBDIR)/$(TARGET)
