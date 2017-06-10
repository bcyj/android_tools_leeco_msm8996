AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic


AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DAEEINTERFACE_CPLUSPLUS \
        -DFEATURE_SMD \
        -DFEATURE_DIAG_MP_MASTER_APPS \
        -DFEATURE_DS_LINUX_ANDROID \
        -DFEATURE_DSS_LINUX \
        -DFEATURE_LINUX \
        -DFEATURE_MULTIPROCESSOR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DFEATURE_NO_REX \
        -DFEATURE_XMLLIB \
        -I../inc \
        $(CFLAGS) \
        $(DIAG_CFLAGS)

requiredlibs = -lpthread -lrt

#####################QCMAP_WEB_PAGES##################

qcmap_web_pagesdir = /WEBSERVER/www/
qcmap_web_pages_DATA = ../pages/QCMAP.html \
                       ../pages/QCMAP_IPV4.html \
                       ../pages/QCMAP_NAT.html \
                       ../pages/QCMAP_Firewall.html \
                       ../pages/QCMAP_WLAN.html \
                       ../pages/QCMAP_IPV6.html \
                       ../pages/QCMAP_Account.html \
                       ../pages/QCMAP_WWAN.html \
                       ../pages/QCMAP_UPNP_DLNA_MDNS.html \
                       ../pages/QCMAP_Help.html \
                       ../pages/QCMAP_IPV4_Help.html \
                       ../pages/QCMAP_IPV6_Help.html \
                       ../pages/QCMAP_NAT_Help.html \
                       ../pages/QCMAP_Firewall_Help.html \
                       ../pages/QCMAP_WLAN_Help.html \
                       ../pages/QCMAP_WWAN_Help.html \
                       ../pages/QCMAP_UPNP_DLNA_MDNS_Help.html \
                       ../pages/QCMAP_Account_Help.html \
                       ../pages/QCMAP_login.html


#####################QCMAP_WEB_IMAGES##################

qcmap_web_imagesdir = /WEBSERVER/www/images/
qcmap_web_images_DATA = ../pages/images/header.jpg \
                        ../pages/images/delete.png \
                        ../pages/images/update.jpg


#####################QCMAP_WEB_CGI##################

qcmap_web_cgidir = /WEBSERVER/www/cgi-bin
qcmap_web_cgi_CC = @CC@
qcmap_web_cgi_SOURCES = qcmap_web_cgi.c

if USE_GLIB
qcmap_web_cgi_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
qcmap_web_cgi_LDFLAGS = -lpthread @GLIB_LIBS@
else
qcmap_web_cgi_CFLAGS = $(AM_CFLAGS) -fPIC
qcmap_web_cgi_LDFLAGS = -lpthread
endif

qcmap_web_cgi_CPPFLAGS = $(AM_CPPFLAGS)
qcmap_web_cgi_LDADD = $(requiredlibs)

qcmap_web_cgi_PROGRAMS = qcmap_web_cgi

#####################QCMAP_AUTH##################

qcmap_authdir = /WEBSERVER/www/cgi-bin

qcmap_auth_CC = @CC@
qcmap_auth_SOURCES = qcmap_auth.c

if USE_GLIB
qcmap_auth_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
qcmap_auth_LDFLAGS = -lpthread -lcrypt @GLIB_LIBS@
else
qcmap_auth_CFLAGS = $(AM_CFLAGS) -fPIC
qcmap_auth_LDFLAGS = -lpthread -lcrypt
endif

qcmap_auth_CPPFLAGS = $(AM_CPPFLAGS)
qcmap_auth_LDADD = $(requiredlibs)

qcmap_auth_PROGRAMS = qcmap_auth
