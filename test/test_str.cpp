#include <cstdint>
#include <cstdio>
#include <cassert>

#include "hice/str.hpp"


void test_append_printf(bool with_nulls) {
	const int N = 10;
	hc::growingstr s;
	for (int i=0; i<N; ++i) {
		s.printf(with_nulls, "%d", i);
		//s.printf2(false, i);
	}
	int step = with_nulls + 1;
	assert(N*step == int(s.length()));
	assert(N*step <= int(s.capacity()));
	for (int i=0; i < N; ++i) {
		assert(*(s.ptr()+i*step) == ('0'+i));
	}
}

void recv(hc::growingstr s) {
	assert(s.ptr());
}

int main() {
	test_append_printf(false);
	test_append_printf(true);
	recv(hc::growingstr(10));
	hc::growingstr a(10);
	recv(a.move());
	assert(!a.ptr());
	a.append('a').append('h');
	a.fdwrite();
	printf("huihui");
}
