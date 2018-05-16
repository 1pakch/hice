#include <unistd.h>
#include <cstdio>
#include <thread>
#include <memory>

#include <hice/pipe.hpp>

using namespace hc;
using namespace hc::pipe;


void* produce(Producer<int> p) {
	printf("P: started\n");
	printf("P: mainloop\n");
	for (size_t i = 0; i < 99; ++i) {
		p.push(i);
	}
	printf("P: ending\n");
	return nullptr;
}

void* transform(Consumer<int> c, Producer <int> p) {
	printf("T: mainloop\n");
	size_t n;
	while ((n = c.recv())) {
		printf("T: received %zd items\n", n);
		for (size_t i = 0; i < n; ++i) {
			int x = c.pop(i);
			p.push(x*2);
		}
	}
	p.send();
	printf("T: ending\n");
	return nullptr;
}


void* consume(Consumer<int> c) {
	printf("C: mainloop\n");
	size_t n;
	while ((n = c.recv())) {
		printf("C: received %zd items\n", n);
	}
	printf("C: ended\n");
	return nullptr;
}

int main() {
	printf("M: Creating pipes\n");
	Pipe<int> pipes[2] = {Pipe<int>(0), Pipe<int>(0)};

	printf("M: Starting producer\n");
	auto p = std::thread(produce, pipes[0].create_producer(10));
	printf("M: Starting transformer\n");
	auto t = std::thread(transform, pipes[0].create_consumer(10), pipes[1].create_producer(10));
	printf("M: Starting consumer\n");
	auto c = std::thread(consume, pipes[1].create_consumer(10));
	p.join();
	pipes[0].close();
	pipes[1].close();
	printf("M: P joined\n");
	printf("M: Closed pipes\n");
	t.join();
	printf("M: T joined\n");
	c.join();
	printf("M: C joined\n");
	return 0;
}
