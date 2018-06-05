#include <string>
#include <sstream>
#include <cstdio>
#include <iostream>

#include <minimap2/minimap.h>
#include <mm2xx/mapped.hpp>
#include <mm2xx/encoding.hpp>
#include <mm2xx/cswriter.hpp>


extern uint8_t seq_nt4_table[256]; // encoding from 'ACGT' to internal

namespace mm {

const std::string empty_string_;

#pragma pack(push,1)
template<class Mapped=Mapped<>>
class Aligned : public Mapped {
    using query_size_type = typename Mapped::query_size_type;
    query_size_type qlen_ = 0;  // Length of aligned part of the query
    query_size_type ldist_= 0; // Levenshtein distance
    std::unique_ptr<std::string> cs_;
    std::unique_ptr<std::string> qs_str_; // Unaligned start of the query in positive ref direction
    std::unique_ptr<std::string> qe_str_; // Unaligned end of the query in positive ref direction
  public:
    Aligned()
        : Mapped() {}

    Aligned(const mm_reg1_t *reg)
        : Mapped(reg)
    {}

    Aligned(Aligned&&) = default;
    Aligned& operator=(Aligned&&) = default;
    Aligned(const Aligned&) = delete;
    Aligned& operator=(const Aligned&) = delete;

    size_t ldist() const { return ldist_; }

    size_t qstart() const { return qs_str_? qs_str_->size() : 0; }
    size_t qend() const { return qstart() + qlen(); }
    size_t qlen() const { return qlen_; }

    const std::string& cs() const { return cs_? *(cs_.get()) : empty_string_; }
    const std::string& qs_str() const { return qs_str_? *(qs_str_.get()) : empty_string_; }
    const std::string& qe_str() const { return qe_str_? *(qe_str_.get()) : empty_string_; }

    void set_ldist(size_t ldist) {
        ldist_ = ldist;
    }

    void set_qlen(size_t qlen) {
        qlen_ = qlen;
    }

    void set_cs(std::string&& cs) {
        cs_ = std::make_unique<std::string>(std::move(cs));
    }

    void set_qs(std::string&& qs) {
        qs_str_ = std::make_unique<std::string>(std::move(qs));
    }

    void set_qe(std::string&& qe) {
        qe_str_ = std::make_unique<std::string>(std::move(qe));
    }


};
#pragma pack(pop)


class AlignerBase: public MapperBase {

    protected:
	std::basic_string<enc::encoded> ref_buf_;
	std::basic_string<enc::encoded> qbuf_;

	using MapperBase::MapperBase;

	// Encode the whole query into the query buffer 
	void load_ref(uint8_t rid, uint32_t start, uint32_t end) {
		ref_buf_.reserve(end - start);
		mm_idx_getseq(idx(), rid, start, end, &ref_buf_[0]);
	}

};


/// Aligner returning the best alignment for a given query
class Aligner: public AlignerBase {
  public:
    Aligner(const Settings& settings): AlignerBase(settings, 1) {}  
    Aligned<> map(std::string const& query) {
        auto hits = MapperBase::map(query);
        if (hits.size()) {
            auto hit = hits[0];
            //printf("rev=%d, qs=%d, qe=%d, qlen=%zd\n", hit.rev, hit.qs, hit.qe, qlen);
            //fflush(stdout);
            // aligned query coordinates relative to positive strand in ref
            size_t qs = hit.rev ? query.size() - hit.qe : hit.qs;
            size_t qe = hit.rev ? query.size() - hit.qs : hit.qe;
            Aligned<> result(&hit); 
            enc::encode_str(qbuf_, query, hit.rev);
            load_ref(hit.rid, hit.rs, hit.re);
            CigarIterator ci(&ref_buf_[0], &qbuf_[qs], hit.p);
            CsStringWriter csw;
            ci.map(csw);
            result.set_ldist(ci.ldist());
            result.set_qlen(ci.qlen());
            assert(qe == qs + ci.qlen()); 
            if (ci.ldist())
                result.set_cs(std::move(csw.result()));
            if (qs != 0)
                result.set_qs(std::move(enc::decode_str(qbuf_, 0, qs)));
            if (qe != query.size())
                result.set_qe(std::move(enc::decode_str(qbuf_, qe, query.size())));
            return std::move(result);
        } else {
            return Aligned<>();
        }
    }
};
// 12-345-6
// 6-543-21

} // namespace mm
