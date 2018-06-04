// enconding.hpp - conversions to and from minimap2 internal representation

#pragma once

#include <cassert>
#include <minimap.h>


/// Table mapping ASCII nucleotides to internal minimap2 representation
extern uint8_t seq_nt4_table[256]; // link with libminimap2.a

namespace mm {

namespace enc {

using ascii = char;            //< Nucleotide as an ASCII character
using encoded = unsigned char; //< Encoded nucleotide
using charidx = unsigned char; //< Unsigned char safe as indexer

constexpr encoded *table_encode = seq_nt4_table; //< Alias for seq_nt4_table
constexpr ascii table_decode[] = "acgtn"; //< Mapping from internal to ascii

static inline encoded encode(ascii c) { return table_encode[(charidx)c]; }
static inline ascii decode(encoded bc) { return table_decode[bc]; }
static inline encoded complement(encoded c) { return c >= 4 ? 4 : 3 - c; }

//< Verify that encoding/decoding works
void encoding_check() {
    for (size_t i = 0; i < 5; ++i) {
        char x = table_decode[i];
        assert(decode(encode(x)) == x);
    }
}

void encode_str(std::basic_string<encoded>& dst,
                const std::basic_string<ascii>& src,
                bool rc) {
    if (dst.capacity() < src.size()) {
        dst.reserve(src.size());
        dst.resize(src.size());
    }
    auto pdst = &dst[0];
    auto start = &src[0];
    auto end = start + src.size();
    if (!rc) {
        while (start < end)
            *pdst++ = encode(*start++);
    } else {
        auto len = end - start;
        pdst += len - 1;
        while (start < end)
            *pdst-- = complement(encode(*start++));
    }
    dst.resize(src.size());
}

std::basic_string<encoded> encode_str(const std::string& query, bool rc) {
    std::basic_string<encoded> result;
    encode_str(result, query, rc);
    return std::move(result);
}

void decode_str(std::basic_string<ascii>& dst,
                const std::basic_string<encoded>& src,
                size_t istart=0, size_t iend=0) {
    if (!iend) iend = src.size();
    if (dst.capacity() < iend - istart) {
        dst.reserve(iend - istart);
    }
    dst.resize(iend - istart);
    auto pdst = &dst[0];
    auto start = src.data() + istart;
    auto end = src.data() + iend;
    while (start < end)
       *pdst++ = decode(*start++);
}

std::basic_string<ascii> decode_str(const std::basic_string<encoded>& src,
                                    size_t istart=0, size_t iend=0) {
    std::basic_string<ascii> result;
    decode_str(result, src, istart, iend);
    return std::move(result);
}


} // namespace enc

} // namespace mm
