all: bin/hice-map bin/pipes

CFLAGS=-std=c99 -Wall -g
INC=-Iminimap2 -Ipipe -Iinclude
CC=gcc

MM2LIB=minimap2/libminimap2.a
HEADERS=$(wildcard include/*.h)

minimap2/libminimap2.a: $(wildcard minimap2/*.h) $(wildcard minimap2/*.c)
	cd minimap2 && make

.SECONDEXPANSION:
bin/%: ${HEADERS} pipe minimap2 ${MM2LIB} src/$$(@F).c
	mkdir -p bin
	${CC} ${CFLAGS} ${INC} src/$(@F).c ${MM2LIB} pipe/pipe.c -lm -lpthread -lz -o $@

test: bin/hice-map
	cd testdata && ${PWD}/$< chrM.fa read1.fa read2.fa

memcheck: bin/hice-map
	cd testdata && valgrind \
		--verbose \
		--show-leak-kinds=all \
		--leak-check=full \
		--track-origins=yes \
		--log-file=../hice-map.valgrind.log \
		${PWD}/bin/hice-map chrM.fa read1.fa read2.fa
