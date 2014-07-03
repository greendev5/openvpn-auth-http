CC=gcc
INCL=
LDFLAGS=-shared
LIBS= -lpthread -lcurl -lc
CFLAGS=-Wall -fPIC -DPIC

SOURCES=auth_http_plugin.c \
		deferred_handlers.c \
		deferred_queue.c \
		plugin_options.c \
		plugin_utils.c\

OBJECTS=auth_http_plugin.o \
		deferred_handlers.o \
		deferred_queue.o \
		plugin_options.o \
		plugin_utils.o\

PLUGIN=openvpn-auth-http.so

all: $(SOURCES) $(PLUGIN)

$(PLUGIN): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

auth_http_plugin.o:
	$(CC) -c auth_http_plugin.c $(INCL) $(CFLAGS) -o auth_http_plugin.o

deferred_queue.o:
	$(CC) -c deferred_queue.c $(INCL) $(CFLAGS) -o deferred_queue.o

deferred_handlers.o:
	$(CC) -c deferred_handlers.c $(INCL) $(CFLAGS) -o deferred_handlers.o

plugin_options.o:
	$(CC) -c plugin_options.c $(INCL) $(CFLAGS) -o plugin_options.o

plugin_utils.o:
	$(CC) -c plugin_utils.c $(INCL) $(CFLAGS) -o plugin_utils.o

clean:
	rm -rf $(PLUGIN) *.o */*.o