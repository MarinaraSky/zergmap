CC = gcc
PROGRAMS = encode decode
HEADERS = src/zergProtos.h src/zergStructs.h
CFILES =  src/zergProtos.o src/graph/Graph.o
CFLAGS = $(CF) -Wall -Wextra -Wpedantic -Wstack-usage=1024 -Wwrite-strings -Wfloat-equal -Waggregate-return -Winline 
DEBUG_FLAGS = -g3

codec: $(PROGRAMS)

encode: src/encode.c  $(CFILES) 
	$(CC) $(CFLAGS) src/encode.c $(CFILES) -o encode -lm

decode: src/decode.c $(CFILES)
	$(CC) $(CFLAGS) -o decode src/decode.c $(CFILES) -lm

debug: src/decode.c src/encode.c $(CFILES)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o decode src/decode.c $(CFILES) -lm
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o encode src/encode.c $(CFILES) -lm
clean:
	@rm -f $(PROGRAMS) *.o
