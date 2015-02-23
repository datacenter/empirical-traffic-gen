CC=g++
CFLAGS=-c -Wall -O2 -pthread -MMD -MP
LDFLAGS=-pthread
COMMON_OBJS=common.o
SERVER_OBJS=server.o
CLIENT_OBJS=ranvar.o client.o
OBJS=$(COMMON_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS)
DEPS=$(OBJS:.o=.d)
TARGETS=server client

all: $(TARGETS)

server: $(COMMON_OBJS) $(SERVER_OBJS)
	$(CC) $(LDFLAGS) -o server $^

client: $(COMMON_OBJS) $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) -o client $^

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.cpp 
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -f *.o *.d $(TARGETS) 

-include $(DEPS)

