TEST_TARGETS=$(shell ls test | sed 's|\([a-zA-Z_]\+\).cpp|bin/test/\1|')
all: ${TEST_TARGETS}

CXFLAGS=-std=c++14 -Wall
DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS+=-O0 -g -DDEBUG
else
    CXXFLAGS+=-O3 -DNDEBUG
endif

INCLUDE=-I${CURDIR} -I${CURDIR}/minimap2 -I${CURDIR}/bpcqueue
CXX=g++ ${CXXFLAGS} ${INCLUDE}

HEADERS=$(wildcard include/*.hpp) $(wildcard mm2xx/*.hpp)

MM2LIB=minimap2/libminimap2.a
${MM2LIB}: $(wildcard minimap2/*.h) $(wildcard minimap2/*.c)
	cd minimap2 && make

LINK=-lm -lpthread -lz ${MM2LIB}
OBJS=

bin/test/%: test/%.cpp ${HEADERS} ${OBJS} ${MM2LIB}
	mkdir -p bin/test
	${CXX} $< ${OBJS} ${LINK} -o $@

bin/%: src/%.cpp ${HEADERS} ${OBJS} ${MM2LIB}
	mkdir -p bin
	${CXX} $< ${OBJS} ${LINK} -o $@

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
