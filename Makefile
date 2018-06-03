TEST_TARGETS=$(shell ls test | sed 's|\([a-zA-Z_]\+\).cpp|bin/test/\1|')
all: ${TEST_TARGETS}

INCLUDE=-I${CURDIR} -I${CURDIR}/minimap2 -I${CURDIR}/bpcqueue
CXX=g++ -g -O0 -Wall ${INCLUDE}

HEADERS=$(wildcard include/*.hpp) $(wildcard mm2xx/*.hpp)

MM2LIB=minimap2/libminimap2.a
MM2LIB: $(wildcard minimap2/*.h) $(wildcard minimap2/*.c)
	cd minimap2 && make

LIBS=-lm -lpthread -lz ${MM2LIB}

OBJS=

bin/test/%: test/%.cpp ${HEADERS} ${OBJS}
	mkdir -p bin/test
	${CXX} $< ${OBJS} ${LIBS} -o $@

#src/%.o: src/%.cpp hice/%.hpp ${HEADERS}
#	${CXX} ${CXFLAGS} ${INC} -c -o $@ $<

.PHONY: clean
clean:
	rm -rf bin

memcheck: bin/hice-map
	cd testdata && valgrind \
		--verbose \
		--show-leak-kinds=all \
		--leak-check=full \
		--track-origins=yes \
		--log-file=../hice-map.valgrind.log \
		${PWD}/bin/hice-map chrM.fa read1.fa read2.fa
