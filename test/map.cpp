
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <bpcqueue.hpp>

#include <mm2xx/mappers.hpp>

#include <hice/gzstream.hpp>


#define NWORKERS 2


using namespace mm;
using namespace hc;
using namespace bpcqueue;


void split_lines(In<std::string> chunks, Out<std::string> lines,
                 size_t take_every=1, size_t skip=0) {
    std::string buf;
    std::string tail;
    size_t lineno = 0;
    size_t start = 0;
    auto take_this_line = [&](){
        return lineno >= skip && (lineno - skip) % take_every; }
    while (bufs.pop(buf)) {
        for (size_t i=0; i<buf.size(); ++i) {
            if (buf[i] == '\n') {
                if (tail) {
                    if (take_this_line()) {
                        auto line = std::move(tail);
                        line.append(buf, start, i);
                        lines.push(line);
                    }
                    tail = std::string();
                } else {
                    if (take_this_line())
                        lines.emplace(buf, start, i);
                }
                ++lineno;
            }
        }
        tail = std::string(buf, start, i);
    }
}

void parse_fasta(In<std::string> chunks, Producer<std::string> sequences) {
    std::string chunk;
    std::string tail;
    size_t start = 0;
    size_t lineno = 0;
    while (chunks.pop(chunk)) {
        
    }
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

	Queue<std::string> chunks(4);
	Queue<std::string> sequences(4);

	auto reader = std::thread(gzstream, output(chunks), gzfile(argv[1], "r", 4096), 4096);
	auto parser = std::thread(parse_fasta, input(chunks), output(sequences));

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
