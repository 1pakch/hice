all: test/test_map_ctx

CFLAGS=-std=c99 -Wall -g
INC=-I${CURDIR} -I${CURDIR}/minimap2 -I${CURDIR}/fmt/include
CC=gcc
CX=g++ -O3 -std=c++17

MM2LIB=minimap2/libminimap2.a
FMTLIB=fmt/libfmt.a
HEADERS=$(wildcard include/*.h) $(wildcard include/*.hpp)

MM2LIB: $(wildcard minimap2/*.h) $(wildcard minimap2/*.c)
	cd minimap2 && make

.SECONDEXPANSION:
bin/%: ${HEADERS} pipe minimap2 ${MM2LIB} src/$$(@F).c
	mkdir -p bin
	${CC} ${CFLAGS} ${INC} src/$(@F).c ${MM2LIB} pipe/pipe.c -lm -lpthread -lz -o $@

runtest: bin/hice-map
	cd testdata && ${PWD}/$< chrM.fa read1.fa read2.fa

OBJS=hice/map_ctx.o pipe/pipe.o
LIBS=-lm -lpthread -lz ${MM2LIB}

pipe/pipe.o: pipe/pipe.c pipe/pipe.h
	${CC} ${CFLAGS} ${INC} -c -o $@ $<

hice/%.o: hice/%.c hice/%.h ${HEADERS}
	${CC} ${CFLAGS} ${INC} -c -o $@ $<

test/%: test/%.c hice/map_ctx.o ${HEADERS} ${OBJS}
	mkdir -p bin
	${CC} ${CFLAGS} ${INC} $< ${OBJS} ${LIBS} -o $@

bin/test_%: test/test_%.cpp hice/map_ctx.o ${HEADERS} ${OBJS}
	mkdir -p bin
	${CX} ${CXFLAGS} ${INC} $< ${OBJS} ${LIBS} -o $@

clean:
	rm -rf hice/*.o
	rm -rf pipe/pipe.o
	#cd test && (ls | grep -v -P '.c$' | xargs rm)	

memcheck: bin/hice-map
	cd testdata && valgrind \
		--verbose \
		--show-leak-kinds=all \
		--leak-check=full \
		--track-origins=yes \
		--log-file=../hice-map.valgrind.log \
		${PWD}/bin/hice-map chrM.fa read1.fa read2.fa
