#pragma once

#include <cstdint>
#include <minimap.h>


namespace mm {


#pragma pack(push,1)
template<typename query_size_t = uint8_t,
         typename chromosome_index_t = uint8_t,
         typename chromosome_size_t = uint32_t>
class Mapped {
    chromosome_index_t rid1_; // 1-based chromosome index

    chromosome_size_t rs_; // start of the alignment in the reference
    query_size_t rlen_;   // length of the alignment in the reference

    uint8_t mapq_ : 6,    // mapping quality
            rc_ : 1,      // if the query is reverse complimented in the alignment
            primary_ : 1; // if the alignment was marked as primary

  public:
    using query_size_type = query_size_t;
    using chromosome_index_type = chromosome_index_t;
    using chromosome_size_type = chromosome_size_t;

    size_t rid1() const { return rid1_; }
    bool is_mapped() const { return rid1(); }
    operator bool() const { return is_mapped(); }

    size_t rstart() const { return rs_; }
    size_t rend() const { return rs_ + rlen_; }
    size_t rlen() const { return rlen_; }

    size_t mapq() const { return mapq_; }
    bool is_rc() const { return rc_; }
    bool is_primary() const { return primary_; }


    Mapped()
        : rid1_(0) {}
    Mapped(const mm_reg1_t *reg)
        : rid1_(reg->rid + 1)
        , rs_(reg->rs)
        , rlen_(reg->re - reg->rs)
        , mapq_(reg->mapq)
        , rc_(reg->rev)
        , primary_(reg->id == reg->parent) {}

    Mapped(Mapped&&) = default;
    Mapped& operator=(Mapped&&) = default;
};
#pragma pack(pop)


} // namespace mm
