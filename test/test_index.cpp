#include <hice/minimap.hpp>
#include <hice/align.hpp>
#include <iostream>
#include <cassert>

char seq[] = "GATCACAGGTCTATCACCCTATTAACCACTCACGGGAGCTCTCCATGCATCAACCAAACCCCAAAGACACCCCCCACAGTTTATGTAGCTTACCTCCTCA";
using namespace mm;

int main() {
	auto s = Settings("sr");
	s.set_reference("testdata/ref.fa");
	auto m = Mapper(s);
	auto a(m.map(std::string(seq)));
	std::cout << (int) a.rid1() << ' ' << (int) a.rstart() << ' ' << (int) a.rend() << ' ' << (int) a.mapq() << '\n';
	//std::cout << a.cs() << '\n';
	assert(a.rid1() == 1);
	assert(a.rstart() == 0);
	assert(a.rend() == 100);
	//for (auto x: a) std::cout << x;
	return 0;

}
