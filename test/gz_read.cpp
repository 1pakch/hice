#include <cstdio>
#include <string>

#include "hice/gzfile.hpp"
#include "hice/fastx.hpp"
#include "hice/timeit.hpp"

#include <ctime>
#include <sstream>


using namespace hc;


size_t countlines(const char *file, size_t bufsize, size_t bytes_to_read) {
	buffered_reader r(file, bufsize);
	size_t bytes_read = 0;
	size_t lines_read = 0;
	while (!r.eof() && bytes_read < bytes_to_read) {
		std::string g(r.getline(128));
		bytes_read += g.size() + 1;
		lines_read += 1;
	}
	return lines_read;
}

size_t skiplines(const char *file, size_t bufsize, size_t lines_to_read) {
	buffered_reader r(file, bufsize);
	size_t lines_read = 0;
	while (!r.eof() && lines_read < lines_to_read) {
		r.skipline();
		lines_read += 1;
	}
	return lines_read;
}

int main(int argc, const char **argv) {
	if (argc != 2) {
		printf("Usage: %s GZIPPED_OR_PLAIN_TEXT_FILE\n", argv[0]);
		return -1;
	}
	const size_t Kb = 2 << (10-1);
	const size_t Mb = 2 << (20-1);
	const size_t Gb = 2 << (30-1);
	const size_t bytes_to_read = Gb / 4;
	for (size_t bufsize = 64*Kb; bufsize < 8*Mb; bufsize <<= 1) {
		auto tc = TIMEIT(countlines, argv[1], bufsize, bytes_to_read).print();
		auto lines_to_read = tc.result();
		TIMEIT(skiplines, argv[1], bufsize, lines_to_read).print();
		printf("%zd lines read\n", lines_to_read);
	}
}
