#include <cstdint>
#include <cstring>
#include <cstdio>

#include "hice/buffer.hpp"
#include "hice/buffer.hpp"

using buftype = genlib::buf<char, genlib::mallocator>;

static const char msg[] = "Hello from transferred buffer\n";

auto create_buf() {
	buftype buf(strlen(msg)+1);
	strcpy(buf.ptr(), msg);
	return buf.move();
}

struct recv_by_value {
	static void recv(buftype buf) {
		assert(!buf.null());
		assert(0 == strcmp(buf.ptr(), msg));
	}
};

struct recv_by_rvalue {
	static void recv(buftype&& buf_) {
		buftype buf(buf_.move());
		assert(!buf.null());
		assert(0 == strcmp(buf.ptr(), msg));
	}
};

template<typename recv>
void test_pass_rvalue() {
	recv::recv(create_buf());
}

template<typename recv>
void test_pass_lvalue() {
	auto buf = create_buf();
	recv::recv(buf.move());
	assert(buf.null());
}

void test_assign() {
	buftype a{};
	buftype b(create_buf());
	a = b.move();
	assert(b.null());
};

int main() {
	test_pass_rvalue<recv_by_value>();
	test_pass_rvalue<recv_by_rvalue>();
	test_assign();
}
