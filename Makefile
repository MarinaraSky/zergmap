CC = gcc
PROGRAMS = zergmap
HEADERS = src/zerg/zergProtos.h src/zerg/zergStructs.h
CFILES =  src/zerg/zergProtos.o src/graph/Graph.o src/map/vmap.o src/map/map.o src/dpqueue/pqueue.o src/dijkstra/Dijkstra.o
CFLAGS = $(CF) -Wall -Wextra -Wpedantic -Wstack-usage=1024 -Wwrite-strings -Wfloat-equal -Waggregate-return -Winline 
LDLIBS = -lcrypto -lm
DEBUG_FLAGS = -g

zergmap: src/zergmap.c $(CFILES)
	$(CC) $(CFLAGS) -o $@ src/zergmap.c $(CFILES) $(LDLIBS)

debug: src/zergmap.c $(CFILES)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $(PROGRAMS) src/zergmap.c $(CFILES) $(LDLIBS)

profile: CFLAGS += -pg
profile: src/zergmap.c $(CFILES)
	$(CC) $(CFLAGS) -o $(PROGRAMS) src/zergmap.c $(CFILES) $(LDLIBS)

clean:
	@rm -f $(PROGRAMS) $(CFILES) *.o gmon.out
