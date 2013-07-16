CC = g++
CFLAGS = -Wall -pthread
LINKFLAGS = -lm

SOURCES = ranvar.cc client.c
HEADERS = ranvar.h client.h
TARGET = client

.PHONY: all clean tags submit

objects = $(SOURCES:.c=.o)

proxy: $(objects) 
	$(CC) $(CFLAGS) -o $(TARGET) $(objects) $(LINKFLAGS)

all: $(TARGET)

clean: 
	@rm -f *.o $(TARGET) 

tags:
	@ctags $(SOURCES)

submit: clean
	@tar zcf pa1.tar.gz $(SOURCES) $(HEADERS) README Makefile

