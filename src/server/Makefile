CC = gcc
CFLAGS = -Wall -g -std=gnu99

SOURCES = server.c
HEADERS = server.h
TARGET = server

.PHONY: all clean tags submit

objects = $(SOURCES:.c=.o)

proxy: $(objects) 
	$(CC) $(CFLAGS) -o $(TARGET) $(objects)

all: $(TARGET)

clean: 
	@rm -f *.o $(TARGET) 

tags:
	@ctags $(SOURCES)

submit: clean
	@tar zcf pa1.tar.gz $(SOURCES) $(HEADERS) README Makefile

