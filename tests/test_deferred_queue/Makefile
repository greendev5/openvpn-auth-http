CC=gcc
INCL=-I../../
LDFLAGS=
LIBS= -lpthread
CFLAGS=-Wall -fPIC -DPIC
SOURCES=main.c ../../deferred_queue.c ../../plugin_utils.c
OBJECTS=deferred_queue.o main.o plugin_utils.o
EXECUTABLE=deferred_queue

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

deferred_queue.o:
	$(CC) -c ../../deferred_queue.c $(INCL) $(CFLAGS) -o deferred_queue.o

main.o:
	$(CC) -c main.c $(INCL) $(CFLAGS) -o main.o

plugin_utils.o:
	$(CC) -c ../../plugin_utils.c $(INCL) $(CFLAGS) -o plugin_utils.o

clean:
	rm -rf $(EXECUTABLE) *.o */*.o