CC=gcc
INCL=-I../../
LDFLAGS=
LIBS= -lpthread -lcurl
CFLAGS=-Wall -fPIC -DPIC
SOURCES=main.c ../../deferred_handlers.c ../../plugin_utils.c
OBJECTS=deferred_handlers.o main.o plugin_utils.o
EXECUTABLE=deferred_handlers

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

deferred_handlers.o:
	$(CC) -c ../../deferred_handlers.c $(INCL) $(CFLAGS) -o deferred_handlers.o

plugin_utils.o:
	$(CC) -c ../../plugin_utils.c $(INCL) $(CFLAGS) -o plugin_utils.o

main.o:
	$(CC) -c main.c $(INCL) $(CFLAGS) -o main.o

clean:
	rm -rf $(EXECUTABLE) *.o */*.o