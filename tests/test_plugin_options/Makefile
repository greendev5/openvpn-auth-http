CC=gcc
INCL=-I../../
LDFLAGS=
LIBS= -lpthread
CFLAGS=-Wall -fPIC -DPIC
SOURCES=main.c ../../plugin_options.c
OBJECTS=plugin_options.o main.o
EXECUTABLE=plugin_options

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

plugin_options.o:
	$(CC) -c ../../plugin_options.c $(INCL) $(CFLAGS) -o plugin_options.o

main.o:
	$(CC) -c main.c $(INCL) $(CFLAGS) -o main.o

clean:
	rm -rf $(EXECUTABLE) *.o */*.o