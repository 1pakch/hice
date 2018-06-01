all: test/test_map_ctx

CFLAGS=-Wall -g
INC=-I${CURDIR} -I${CURDIR}/minimap2 -I${CURDIR}/fmt/include
CC=gcc
CX=g++ -g -O0 -Wall

HEADERS=$(wildcard include/*.h) $(wildcard include/*.hpp) $(wildcard mm2xx/*.hpp)

OBJS=pipe/pipe.o
pipe/pipe.o: pipe/pipe.c pipe/pipe.h
	${CC} ${CFLAGS} ${INC} -c -o $@ $<

LIBS=-lm -lpthread -lz ${MM2LIB}
MM2LIB=minimap2/libminimap2.a
MM2LIB: $(wildcard minimap2/*.h) $(wildcard minimap2/*.c)
	cd minimap2 && make

#hice/%.o: hice/%.c hice/%.h ${HEADERS}
#	${CC} ${CFLAGS} ${INC} -c -o $@ $<

#test/%: test/%.c hice/map_ctx.o ${HEADERS} ${OBJS}
#	mkdir -p bin
#	${CC} ${CFLAGS} ${INC} $< ${OBJS} ${LIBS} -o $@

bin/test/%: test/%.cpp ${HEADERS} ${OBJS}
	mkdir -p bin/test
	${CX} ${CXFLAGS} ${INC} $< ${OBJS} ${LIBS} -o $@

#src/%.o: src/%.cpp hice/%.hpp ${HEADERS}
#	${CX} ${CXFLAGS} ${INC} -c -o $@ $<

.PHONY: clean
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
