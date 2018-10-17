CC = gcc
PROGRAMS = encode decode
HEADERS = src/zerg/zergProtos.h src/zerg/zergStructs.h
CFILES =  src/zerg/zergProtos.o src/graph/Graph.o src/map/vmap.o src/map/map.o src/dpqueue/pqueue.o src/dijkstra/Dijkstra.o
CFLAGS = $(CF) -Wall -Wextra -Wpedantic -Wstack-usage=1024 -Wwrite-strings -Wfloat-equal -Waggregate-return -Winline 
LDLIBS += -lcrypto
DEBUG_FLAGS = -g3

codec: $(PROGRAMS)

encode: src/encode.c  $(CFILES)
	$(CC) $(CFLAGS) src/encode.c $(CFILES) -o encode -lm $(LDLIBS)

decode: src/decode.c $(CFILES)
	$(CC) $(CFLAGS) -o decode src/decode.c $(CFILES) -lm $(LDLIBS)

debug: src/decode.c src/encode.c $(CFILES)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o decode src/decode.c $(CFILES) -lm $(LDLIBS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o encode src/encode.c $(CFILES) -lm $(LDLIBS)
clean:
	@rm -f $(PROGRAMS) *.o src/*/*.o
