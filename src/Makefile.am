
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = openvpn-auth-http.la

openvpn_auth_http_la_SOURCES = \
    auth_http_plugin.c \
    auth_http_plugin.h \
    deferred_handlers.c \
    deferred_handlers.h \
    deferred_queue.c \
    deferred_queue.h \
    openvpn-plugin.h \
    plugin_options.c \
    plugin_options.h \
    plugin_utils.c \
    plugin_utils.h

openvpn_auth_http_la_LIBADD = -lpthread -lcurl -lc

openvpn_auth_http_la_LDFLAGS = \
    -version-info $(LT_VERSION) \
    -release $(LT_RELEASE) \
    -module
