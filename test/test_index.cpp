#include <hice/minimap.hpp>
#include <iostream>

char seq[] = "GATCACAGGTCTATCACCCTATTAACCACTCACGGGAGCTCTCCATGCATTTGGTATTTTCGTCTGGGGGGTGTGCACGCGATAGCATTGCGAGACGCTG";

int main() {
	auto s = mapping::Settings("testdata/ref.fa", "sr", 1); 
	auto m = s.create_mapper();
	auto a(m.map(std::string(seq), 1));
	std::cout << a.size() << '\n';
	for (auto x: a) std::cout << x;
	//return 0;

}
