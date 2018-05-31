#pragma once

#include <cstdint>
#include <minimap.h>


namespace mm {


const std::string empty_string_;

template<typename query_size_t = uint8_t, typename genome_index_t = uint8_t,
         typename reference_size_t = uint32_t>
class Mapped {
    genome_index_t rid1_; // 1-based chromosome index

    reference_size_t rs_; // start of the alignment in the reference
    query_size_t rlen_;   // length of the alignment in the reference

    query_size_t ldist_; // Levenshtein distance

    uint8_t mapq_ : 6, // mapping quality
        rc_ : 1,       // if the query is reverse complimented in the alignment
        primary_ : 1;  // if the alignment was marked as primary

  public:
    genome_index_t rid1() const { return rid1_; }
    bool is_mapped() const { return rid1(); }
    query_size_t rstart() const { return rs_; }
    query_size_t rend() const { return rs_ + rlen_; }
    // query_size_t qlen() const { return qstr_len_; }
    uint8_t mapq() const { return mapq_; }
    bool is_rc() const { return rc_; }
    bool is_primary() const { return primary_; }

    Mapped()
        : rid1_(0) {}
    Mapped(const std::string &query, const mm_reg1_t *reg)
        : rid1_(reg->rid + 1)
        , rs_(reg->rs)
        , rlen_(reg->re - reg->rs)
        //, qstr_len_(query.size())
        , mapq_(reg->mapq)
        , rc_(reg->rev)
        , primary_(reg->id == reg->parent) {}
};


} // namespace mm
