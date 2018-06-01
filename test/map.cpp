
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <mm2xx/mappers.hpp>
#include <include/pipe.hpp>

#define NWORKERS 2


using namespace mm;
using namespace hc;

void parse(Consumer<std::string> c, Producer<std::string> p) {
}

void map(const Settings& s, Consumer<std::string> c, Producer<handle<mm_reg1_t[]>> p) {
	MapperBase m(s, 10);
	while (c) {
		auto sequences = c.recv();
		for (auto seq: sequences) {
			p.push(std::move(m.map(seq)));
		}
	}
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: REFERENCE FASTA\n");
		return -1;
	}

	Settings s("sr");
	s.index_file(argv[1]);

	Pipe<std::string> chunks(4);
	auto p = std::thread(gz_read_into, chunks.create_producer(4), gzfile(argv[1], "r", 4096), 4096);
	auto c = std::thread(consume, chunks.create_consumer(4));

	printf("M: Closed pipes\n");
	chunks.close();
	c.join();
	printf("M: C joined\n");
	printf("M: P joined\n");
	p.join();
	fflush(stdout);
	return 0;
	return 0;
}
