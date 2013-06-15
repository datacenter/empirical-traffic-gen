CC = gcc
CFLAGS = -Wall -g -std=gnu99 -pthread
LINKFLAGS = -lm

SOURCES = client.c
HEADERS = client.h
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

