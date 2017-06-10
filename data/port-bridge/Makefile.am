
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS := port_bridge

port_bridge_CFLAGS := -Wall
port_bridge_CFLAGS += -Wundef
port_bridge_CFLAGS += -Wstrict-prototypes
port_bridge_CFLAGS += -Wno-trigraphs
port_bridge_CFLAGS += -g

port_bridge_CPPFLAGS := -D__packed__=
port_bridge_CPPFLAGS += -DIMAGE_APPS_PROC
port_bridge_CPPFLAGS += -DFEATURE_Q_SINGLE_LINK
port_bridge_CPPFLAGS += -DFEATURE_Q_NO_SELF_QPTR
port_bridge_CPPFLAGS += -DFEATURE_LINUX
port_bridge_CPPFLAGS += -DFEATURE_NATIVELINUX
port_bridge_CPPFLAGS += -DFEATURE_DSM_DUP_ITEMS
port_bridge_CPPFLAGS += -DLE_PORT_BRIDGE_DBG

port_bridge_SOURCES := dun_kevents.c
port_bridge_SOURCES += dun_service.c
port_bridge_SOURCES += port_bridge.c

port_bridge_LDFLAGS := -lpthread
port_bridge_LDFLAGS += -lcutils

