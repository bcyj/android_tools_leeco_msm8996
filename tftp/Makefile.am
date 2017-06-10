LOCAL_CFLAGS  := -Wall -Werror
LOCAL_CFLAGS  += -DFEATURE_TFTP_SERVER_BUILD -DTFTP_LA_BUILD
LOCAL_CFLAGS  += -DTFTP_LE_BUILD_ONLY
LOCAL_CFLAGS  += -DTFTP_ENABLE_DEBUG_MODE
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_ERR_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_WARN_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_DEBUG_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_INFO_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_LINE_NO_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_CONSOLE_LOGGING
LOCAL_CFLAGS  += -g
LOCAL_CFLAGS  += -O0

LOCAL_C_INCLUDES  :=  \
				-Icommon/inc \
				-Ios/inc \
				-Iserver/inc

LOCAL_SRC_FILES  := \
				common/src/tftp_connection.c \
				common/src/tftp_log.c \
				common/src/tftp_malloc.c \
				common/src/tftp_pkt.c \
				common/src/tftp_protocol.c \
				os/src/tftp_file.c \
				os/src/tftp_msg_la.c \
				os/src/tftp_os.c \
				os/src/tftp_os_la.c \
				os/src/tftp_os_wakelocks_la.c \
				os/src/tftp_socket_ipcr_la.c \
				os/src/tftp_string_la.c \
				os/src/tftp_threads_la.c \
				os/src/tftp_utils_la.c \
				server/src/tftp_server.c \
				server/src/tftp_server_folders_la.c \
				server/src/tftp_server_main.c \
				server/src/tftp_server_utils.c

AM_CFLAGS = \
				-Wall \
				-Wundef \
				-Wstrict-prototypes \
				-Wno-trigraphs \
				$(LOCAL_CFLAGS) \
				$(LOCAL_C_INCLUDES)

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D__packed__= \
	$(LOCAL_CFLAGS) \
	$(LOCAL_C_INCLUDES)


bin_PROGRAMS = tftp_server
tftp_server_CC = @CC@
pkgconfigdir = $(libdir)/pkgconfig

tftp_server_SOURCES =  $(LOCAL_SRC_FILES)

tftp_server_LDFLAGS = -lpthread
tftp_server_LDADD = -lpthread

if USE_GLIB
  AM_CPPFLAGS  += -DUSE_GLIB @GLIB_CFLAGS@
  tftp_server_LDFLAGS += @GLIB_LIBS@
endif