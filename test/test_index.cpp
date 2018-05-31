#include <mm2xx/mappers.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <cassert>


using namespace mm;


static const size_t Length = 100;

static const char* ref[] = {
	"GATCACAGGTCTATCACCCTATTAACCACTCACGGGAGCTCTCCATGCATCAACCAAACCCCAAAGACACCCCCCACAGTTTATGTAGCTTACCTCCTCA",
	"TTTTACAGGTCTATCACCCTATTAACCACTCACGGGAGCTCTCCATGCATCAACCAAACCCCAAAGACACCCCCCACAGTTTATGTAGCTTACCTCCTCA"
};

static const std::string seq(
	"GATCACAGGTCTATCACCCTATTAACCACTCACGGGAGCTCTCCATGCATCAACCAAACCCCAAAGACACCCCCCACAGTTTATGTAGCTTACCTCCTCA");


struct ExpectedHitFailure {
	const char* field;
	const int expected;
	const int actual;
};

struct ExpectedHit {
	const int rid1;
	const int rstart;
	const int rend;
	const int mapq;
	static void assert_eq(const char* field, int expected, int actual) {
		if (expected != actual) throw ExpectedHitFailure{field, expected, actual};
	}
	void assert_eq(const Mapped<>& hit) const {
		assert_eq("rid1", rid1, hit.rid1());
		assert_eq("rstart", rstart, hit.rstart());
		assert_eq("rend", rend, hit.rend());
		assert_eq("mapq", mapq, hit.mapq());
	}
};

static const std::map<std::string, std::vector<ExpectedHit>> test_cases = {
	{{ref[0], Length}, {
		{1, 0, 100, 35},
		{2, 4, 100, 0}}}

};

void test_multi_mapper(const Settings &s) {
	auto m = MultiMapper(s, 2);
	size_t case_n = 0;
	for (auto test_case: test_cases) {
		auto seq = test_case.first;
		auto expected_hits = test_case.second;
		size_t hit_n = 0;
		for (auto hit: m.map(std::string(test_case.first))) {
			try {
				expected_hits[hit_n].assert_eq(hit);
			} catch (const ExpectedHitFailure& f) {
				std::cout << "test_multi_mapper() "
					  << "case=" << case_n << " "
					  << "hit=" << hit_n << " "
					  << f.field << " "
					  << "expected=" << f.expected
					  << "actual=" << f.actual
					  << '\n';
			};
			hit_n++;
		}
		case_n++;
	}
}

void test_mapper(const Settings &s) {
	auto m = Mapper(s);
	auto a(m.map(seq));
	size_t case_n = 0;
	for (auto test_case: test_cases) {
		auto seq = test_case.first;
		auto expected_hits = test_case.second;
		size_t hit_n = 0;
		auto hit = m.map(std::string(test_case.first));
		try {
			expected_hits[hit_n].assert_eq(hit);
		} catch (const ExpectedHitFailure& f) {
			std::cout << "test_multi_mapper() "
				  << "case=" << case_n << " "
				  << "hit=" << hit_n << " "
				  << f.field << " "
				  << "expected=" << f.expected
				  << "actual=" << f.actual
				  << '\n';
		};
		case_n++;
	}
	// std::cout << (int) a.rid1() << ' ' << (int) a.rstart() << ' ' << (int) a.rend() << ' ' << (int) a.mapq() << '\n';
}

int main() {
	auto settings = Settings("sr");
	settings.index_strings(2, ref, nullptr);
	
	test_mapper(settings);
	test_multi_mapper(settings);

	return 0;
}
