// enconding.hpp - conversions to and from minimap2 internal representation

#pragma once

#include <minimap.h>
#include <cassert>


/// Table mapping ASCII nucleotides to internal minimap2 representation
extern uint8_t seq_nt4_table[256]; // link with libminimap2.a

namespace mm {

namespace enc {

using ascii = char; //< Nucleotide as an ASCII character
using encoded = unsigned char; //< Encoded nucleotide
using charidx = unsigned char; //< Unsigned char safe as indexer

constexpr encoded *table_encode = seq_nt4_table; //< Alias for seq_nt4_table
constexpr ascii table_decode[] = "acgtn"; //< Mapping from internal to ascii

static inline encoded encode(ascii c){ return table_encode[(charidx) c]; }
static inline ascii decode(encoded bc) { return table_decode[bc]; }
static inline encoded complement(encoded c) { return c >= 4 ? 4 : 3 - c; }

//< Verify that encoding/decoding works
void encoding_check() {
	for (size_t i=0; i<5; ++i) {
		char x = table_decode[i];
		assert(decode(encode(x)) == x);
	}
}

static void encode_str(encoded *dst, const ascii *start, const ascii *end,
		       bool reverse_complement) {
	if (!reverse_complement) {
		while (start < end)
			*dst++ = encode(*start++);
	} else {
		auto len = end - start;
		dst += len - 1;
		while (start < end)
			*dst-- = complement(encode(*start++));
	}
}

} // encoding

} // mm
