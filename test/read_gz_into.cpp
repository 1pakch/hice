#include <string>
#include <thread>

#include <hice/pipe.hpp>
#include <hice/gz.hpp>
#include <unistd.h>


using namespace hc;


void* consume(Consumer<std::string> c) {
	printf("C: mainloop\n");
	auto v = c.recv();
	while (v.size()) {
		printf("C: received %zd strings\n", v.size());
		for (auto s: v) {
			printf("C: string %zd = %s\n", s.size(), s.c_str());
		}
		v = c.recv();
	}
	printf("C: ended\n");
	return nullptr;
}

int main(int argc, const char **argv) {
	if (argc != 2) {
		printf("Usage: %s GZIPPED_OR_PLAIN_TEXT_FILE\n", argv[0]);
		return -1;
	}

	printf("M: Creating pipes\n");
	Pipe<std::string> chunks(4);

	printf("M: Starting producer\n");
	auto p = std::thread(gz_read_into, chunks.create_producer(4), gzfile(argv[1], "r", 4096), 4096);
	printf("M: Starting consumer\n");
	auto c = std::thread(consume, chunks.create_consumer(4));

	printf("M: Closed pipes\n");
	chunks.close();
	c.join();
	printf("M: C joined\n");
	printf("M: P joined\n");
	p.join();
	fflush(stdout);
	return 0;
}
